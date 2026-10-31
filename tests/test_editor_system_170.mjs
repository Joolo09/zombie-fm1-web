/* SPDX-License-Identifier: MIT — the web editor reads and writes the 0.16/0.17 system settings the firmware keeps. */
import assert from 'node:assert/strict';import {create} from './native_emulator_harness.mjs';import {protocol} from './native_protocol_harness.mjs';
import {decodeSystem,SYSTEM_KEYS,SYSTEM_MAX,systemDefaults} from '../webapp/groovebox-180/system.js';
let failed=0;const check=(name,fn)=>{try{fn();console.log('PASS '+name);}catch(e){failed++;console.error('FAIL '+name+': '+e.message);}};

check('a fresh device reads as the defaults: 1-bar count-in, click on, clock out off, no scale',()=>{const e=create(),p=protocol(e);const s=decodeSystem(p(28));
 assert.equal(s.countin,1);assert.equal(s.recClick,1);assert.equal(s.clockOut,0);assert.equal(s.scale,0);assert.equal(s.root,0);});

check('every setting written with command 29 reads back through decodeSystem',()=>{const e=create(),p=protocol(e);
 for(const [id,k] of SYSTEM_KEYS.entries()){if(id===0||id===6||id===7||id===1)continue;for(const v of [SYSTEM_MAX[id],0,Math.min(1,SYSTEM_MAX[id])]){p(29,[id,v]);assert.equal(decodeSystem(p(28))[k],v,k+'='+v);}}});

check('out-of-range values are refused by the firmware (status 1), not stored',()=>{const e=create(),p=protocol(e);
 p(29,[10,10],1);p(29,[11,12],1);p(29,[8,3],1);p(29,[12,2],1);p(29,[20,0],1);const s=decodeSystem(p(28));assert.deepEqual([s.scale,s.root,s.countin,s.clockOut],[0,0,1,0]);});

check('scale and root are independent nibbles',()=>{const e=create(),p=protocol(e);p(29,[10,7]);p(29,[11,9]);let s=decodeSystem(p(28));assert.deepEqual([s.scale,s.root],[7,9]);p(29,[10,2]);s=decodeSystem(p(28));assert.deepEqual([s.scale,s.root],[2,9]);});

check('the editor defaults and limits match the device',()=>{const d=systemDefaults();assert.deepEqual([d.countin,d.recClick,d.scale,d.root,d.clockOut],[1,1,0,0,0]);assert.equal(SYSTEM_KEYS.length,SYSTEM_MAX.length);});

check('an old 0.15 reply (padding zero) decodes to the defaults',()=>{const e=create(),p=protocol(e);const b=Uint8Array.from(p(28));b[110]=0;b[111]=0;const s=decodeSystem(b);assert.deepEqual([s.countin,s.recClick,s.clockOut,s.scale,s.root],[1,1,0,0,0]);});

check('a corrupt scale nibble in a saved file never reaches the keys',()=>{const e=create(),p=protocol(e);const b=Uint8Array.from(p(28));b[111]=0xFF;const s=decodeSystem(b);assert.deepEqual([s.scale,s.root],[0,0]);});

if(failed)process.exit(1);
