/* SPDX-License-Identifier: MIT — original 16-step, four-note piano roll. */
import {DRUMS,drumIndex} from './drums.js';
export function drawPianoRoll(canvas,state,selected,bank,head=-1){
 if(state.selected===5)return drawDrumRoll(canvas,state,selected,bank,head);
 if(!canvas)return;const ctx=canvas.getContext('2d'),w=canvas.width,h=canvas.height,left=42,top=22,cw=(w-left-8)/16,rh=(h-top-6)/24,t=state.tracks[state.selected];
 if(head>=0)head%=t.p[23];
 const base=Math.max(0,Math.min(104,t.steps[selected].note-9));canvas.dataset.base=String(base);canvas.dataset.selected=String(selected);canvas.dataset.drums='0';
 ctx.fillStyle='#080e18';ctx.fillRect(0,0,w,h);ctx.font='11px ui-monospace,monospace';
 for(let row=0;row<24;row++){const note=base+23-row,y=top+row*rh;ctx.fillStyle=[1,3,6,8,10].includes(note%12)?'#0e1a29':'#142333';ctx.fillRect(left,y,w-left-8,rh-1);ctx.fillStyle='#8fa9bc';if(note%12===0||note===t.steps[selected].note)ctx.fillText(['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'][note%12]+(Math.floor(note/12)-1),3,y+rh-2);}
 for(let i=0;i<=16;i++){ctx.fillStyle=i%4?'#213348':'#3b526b';ctx.fillRect(left+i*cw,top,1,h-top-6);if(i<16){ctx.fillStyle='#8fa9bc';ctx.fillText(String(bank*16+i+1),left+i*cw+3,14);}}
 for(let i=0;i<16;i++){const at=bank*16+i,s=t.steps[at];if(!s.velocity)continue;for(const note of [s.note,...s.chord.filter(Boolean).map(n=>n-1)]){if(note<base||note>base+23)continue;ctx.globalAlpha=.55+s.velocity/127*.45;ctx.fillStyle=at===selected?'#edf8fb':'#73efd0';const x=left+i*cw+2,y=top+(base+23-note)*rh+1;ctx.fillRect(x,y,Math.min(w-8-x,(s.duration||1)*cw-4),rh-3);ctx.globalAlpha=1;}}
 if(head>=bank*16&&head<bank*16+16){ctx.fillStyle='#ff8faf';ctx.fillRect(left+(head%16)*cw,top,2,h-top-6);}
}
function drawDrumRoll(canvas,state,selected,bank,head){
 if(!canvas)return;const ctx=canvas.getContext('2d'),w=canvas.width,h=canvas.height,left=110,top=22,cw=(w-left-8)/16,rh=(h-top-6)/16,t=state.tracks[5];
 canvas.dataset.drums='1';canvas.dataset.selected=String(selected);ctx.fillStyle='#080e18';ctx.fillRect(0,0,w,h);ctx.font='11px ui-monospace,monospace';
 for(let row=0;row<16;row++){const d=DRUMS[15-row],y=top+row*rh;ctx.fillStyle=row%2?'#0e1a29':'#142333';ctx.fillRect(left,y,w-left-8,rh-1);ctx.fillStyle=d[0]===t.steps[selected].note?'#edf8fb':'#8fa9bc';ctx.fillText(`${d[0]} ${d[state.language===0?2:1]}`,3,y+rh-3);}
 for(let i=0;i<=16;i++){ctx.fillStyle=i%4?'#213348':'#3b526b';ctx.fillRect(left+i*cw,top,1,h-top-6);if(i<16){ctx.fillStyle='#8fa9bc';ctx.fillText(String(bank*16+i+1),left+i*cw+3,14);}}
 for(let i=0;i<16;i++){const at=bank*16+i,s=t.steps[at];if(!s.velocity)continue;for(const note of [s.note,...s.chord.filter(Boolean).map(n=>n-1)]){const row=15-drumIndex(note);ctx.fillStyle=at===selected?'#edf8fb':'#be90f9';ctx.globalAlpha=.55+s.velocity/127*.45;ctx.fillRect(left+i*cw+3,top+row*rh+2,cw-6,rh-4);ctx.globalAlpha=1;}}
 if(head>=0){head%=t.p[23];if(head>=bank*16&&head<bank*16+16){ctx.fillStyle='#ff8faf';ctx.fillRect(left+(head%16)*cw,top,2,h-top-6);}}
}
export function bindPianoRoll(canvas,{getState,getSelected,getBank,edit,select,status}){
 if(!canvas)return;const hit=e=>{const r=canvas.getBoundingClientRect(),x=(e.clientX-r.left)*canvas.width/r.width,y=(e.clientY-r.top)*canvas.height/r.height,drums=getState().selected===5,left=drums?110:42,row=Math.max(0,Math.min(drums?15:23,Math.floor((y-22)/((canvas.height-28)/(drums?16:24)))));return {at:getBank()*16+Math.max(0,Math.min(15,Math.floor((x-left)/((canvas.width-left-8)/16)))),note:drums?DRUMS[15-row][0]:Math.max(0,Math.min(127,Number(canvas.dataset.base)+23-row))};};let start;
 canvas.onpointerdown=e=>{e.preventDefault();canvas.focus();canvas.setPointerCapture(e.pointerId);start={...hit(e),add:e.shiftKey,remove:e.altKey};};
 canvas.onpointerup=e=>{if(!start)return;const first=start;start=null;const end=hit(e);edit({...first,duration:end.at>first.at?end.at-first.at+1:null});};canvas.onpointercancel=()=>{start=null;};
 canvas.onkeydown=e=>{const at=getSelected(),s=getState().tracks[getState().selected].steps[at];if(e.code==='ArrowLeft'||e.code==='ArrowRight'){e.preventDefault();e.stopPropagation();select(Math.max(0,Math.min(63,at+(e.code==='ArrowRight'?1:-1))));}
 else if(['ArrowUp','ArrowDown','Space','Equal','Minus'].includes(e.code)){e.preventDefault();e.stopPropagation();const d=e.code==='ArrowUp'?1:e.code==='ArrowDown'?-1:0;edit({at,note:getState().selected===5?DRUMS[Math.max(0,Math.min(15,drumIndex(s.note)+d))][0]:Math.max(0,Math.min(127,s.note+d)),duration:e.code==='Equal'?Math.min(63,(s.duration||1)+1):e.code==='Minus'?Math.max(0,(s.duration||1)-1):null,toggle:e.code==='Space'});}};
 canvas.setAttribute('aria-label','Piano roll: arrows select and transpose; Space toggles; + / - changes length');
}
