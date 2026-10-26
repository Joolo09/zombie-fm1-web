// SPDX-License-Identifier: MPL-2.0
// Copyright (C) 2026 Charles Vestal
// An fm1-base app in an AudioWorklet: app.wasm is the whole FM-1 app (web/emu/emu_web.c). Each render
// quantum asks it for 128 frames, which runs the device's clock forward; between quanta the page's
// input goes in and, about 30 times a second, the screen, the lights, any saved objects and what the
// firmware said to a screen reader (fm1-reader's SysEx frames) go out.

const clock = globalThis.performance ? () => globalThis.performance.now() : () => Date.now();

class Fm1 extends AudioWorkletProcessor {
  constructor() {
    super();
    this.ex = null;
    this.input = null;
    this.sentBlits = -1;
    this.sentWrites = -1;
    this.lastFrame = 0;
    this.inputQueue=[];this.inputMasks={buttons:0,keys:0};this.edgeTimes={buttons:new Float64Array(32).fill(-1e9),keys:new Float64Array(32).fill(-1e9)};this.frameCount=0;this.inputBarrier=0;
    this.busy = 0;
    this.frames = 0;
    this.port.onmessage = (e) => this.onMessage(e.data).catch(error=>this.port.postMessage({error:error.message}));
  }

  async onMessage(m) {
    if (m.type === "load") {
      const now = () => BigInt(Math.round(currentTime * 1e9));
      const wasi = {
        clock_time_get: (id, prec, out) => {
          new DataView(this.mem.buffer).setBigUint64(out, now(), true);
          return 0;
        },
        fd_write: (fd, iov, n, out) => {const v=new DataView(this.mem.buffer);let count=0;for(let i=0;i<n;i++)count+=v.getUint32(iov+i*8+4,true);v.setUint32(out,count,true);return 0;},
        proc_exit: () => {},
      };
      const stub = new Proxy(wasi, { get: (t, k) => (k in t ? t[k] : () => 0) });
      const env = new Proxy({}, { get: () => () => 0 });
      const { instance } = await WebAssembly.instantiate(m.wasm, { wasi_snapshot_preview1: stub, env });
      this.ex = instance.exports;
      this.mem = this.ex.memory;
      if (this.ex._initialize) this.ex._initialize();
      const max = this.ex.web_store_max();
      for (const [obj, bytes] of Object.entries(m.store || {})) {
        if(!Number.isInteger(+obj)||+obj<0||+obj>=this.ex.web_nobj()||!Array.isArray(bytes))continue;
        const b = new Uint8Array(bytes).subarray(0, max);
        new Uint8Array(this.mem.buffer, this.ex.web_store(+obj), b.length).set(b);
        this.ex.web_store_set_len(+obj, b.length);
      }
      this.sentWrites = this.ex.web_store_writes();
      this.ex.web_master(m.master ?? 2800);
      this.ex.web_boot();
      this.port.postMessage({ type: "ready" });
    } else if (!this.ex) {
      return;
    } else if(m.type==='release'){
      this.inputQueue=[];this.inputMasks={buttons:0,keys:0};this.inputBarrier=0;this.ex.web_buttons(0);this.ex.web_keys(0);
    } else if(m.type==='reset'||m.type==='buttons'||m.type==='keys'||m.type==='enc'||m.type==='preset'){
      this.inputQueue.push(m);

    } else if (m.type === "master") {
      this.ex.web_master(m.value | 0);
    } else if (m.type === "sysex") {                  // a screen reader's frame: the bytes between F0 and F7
      const b = Uint8Array.from(m.bytes).subarray(0, 64);
      new Uint8Array(this.mem.buffer, this.ex.web_sysex_buf(), b.length).set(b);
      this.ex.web_sysex(b.length);
    }
  }

  publish() {
    const ex = this.ex;
    const blits = ex.web_blits();
    const root=new Uint8Array(this.mem.buffer,ex.z_snapshot(),ex.z_state_size());
    const msg = { type: "frame", stepGrid:!!ex.web_step_grid(),bank:root[1875],octave:(root[1872]<<24)>>24,track:root[9],section:root[1873],step:root[1907],preset:root[1900+root[9]],drum:new Uint8Array(this.mem.buffer,ex.z_dm_data(),8)[6],playing:!!ex.z_playing(),recording:!!new Uint8Array(this.mem.buffer,ex.z_snapshot(),ex.z_state_size())[1906],buttons: ex.web_lit_buttons() >>> 0, keys: ex.web_lit_keys() >>> 0 };
    if (this.frames >= 44100) {                       // the share of real time spent running the device
      msg.load = Math.round(100 * this.busy / (this.frames / 44.1));
      this.busy = this.frames = 0;
    }
    const transfer = [];
    const said = ex.web_said_len();
    if (said) {                                       // whole F0..F7 frames
      msg.said = new Uint8Array(this.mem.buffer, ex.web_said_buf(), said).slice();
      ex.web_said_clear();
      transfer.push(msg.said.buffer);
    }
    if (blits !== this.sentBlits) {
      this.sentBlits = blits;
      msg.fb = new Uint16Array(this.mem.buffer, ex.web_fb(), 240 * 240).slice();
      transfer.push(msg.fb.buffer);
    }
    const writes = ex.web_store_writes();
    if (writes !== this.sentWrites) {
      this.sentWrites = writes;
      msg.store = {};
      for (let o = 0; o < ex.web_nobj(); o++) {
        const n = ex.web_store_len(o);
        if (n >= 0) msg.store[o] = new Uint8Array(this.mem.buffer, ex.web_store(o), n).slice();
      }
    }
    this.port.postMessage(msg, transfer);
  }

  process(inputs, outputs) {
    if (!this.ex) return true;
    const out = outputs[0];
    const n = out[0].length;
    const t0 = clock();
    while(this.inputQueue.length){if(this.frameCount<this.inputBarrier)break;const m=this.inputQueue[0];if(m.type==='reset'){this.inputQueue.shift();this.ex.web_reset();this.inputMasks={buttons:0,keys:0};this.edgeTimes.buttons.fill(-1e9);this.edgeTimes.keys.fill(-1e9);this.sentBlits=-1;continue;}
      if(m.type==='preset'){this.inputQueue.shift();const t=new Uint8Array(this.mem.buffer,this.ex.z_snapshot(),this.ex.z_state_size())[9];if(t<5&&Number.isInteger(m.value)&&m.value>=0&&m.value<96)this.ex.z_preset(t,m.value);continue;}
      if(m.type==='enc'){this.inputQueue.shift();this.ex.web_enc(m.role,m.n|0);continue;}
      const changed=this.inputMasks[m.type]^m.mask;if(!changed){this.inputQueue.shift();continue;}let blocked=false;for(let bit=0;bit<27;bit++)if(changed>>bit&1&&this.frameCount-this.edgeTimes[m.type][bit]<1058)blocked=true;if(blocked)break;
      this.inputQueue.shift();for(let bit=0;bit<27;bit++)if(changed>>bit&1)this.edgeTimes[m.type][bit]=this.frameCount;this.inputMasks[m.type]=m.mask;
      if(m.type==='buttons'){this.ex.web_buttons(m.mask>>>0);this.inputBarrier=this.frameCount+1058;break;}else this.ex.web_keys(m.mask>>>0);
    }
    this.ex.web_render(n);this.frameCount+=n;
    this.busy += clock() - t0;
    this.frames += n;
    out[0].set(new Float32Array(this.mem.buffer, this.ex.web_out_l(), n));
    if (out[1]) out[1].set(new Float32Array(this.mem.buffer, this.ex.web_out_r(), n));
    if (currentTime - this.lastFrame >= 1 / 30) {
      this.lastFrame = currentTime;
      this.publish();
    }
    return true;
  }
}

registerProcessor("fm1", Fm1);
