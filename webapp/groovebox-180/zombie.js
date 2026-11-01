/* SPDX-License-Identifier: MIT — the block zombie of the FM-1 screen, in the editor's sidebar.
 * Display only: it watches the position readout (nods on the beat while the song plays), the output meter (opens
 * its mouth) and the selected track (eye colour). */
const canvas=document.getElementById('zombie');
if(canvas){
 const ctx=canvas.getContext('2d'),S=2,OX=-14,OY=2;
 const rect=(x,y,w,h,c)=>{ctx.fillStyle=c;ctx.fillRect(OX+x*S,OY+y*S,w*S,h*S);};
 const C={dark:'#0c1a11',skin:'#649a3a',shade:'#3c6c2c',light:'#93ba4e',shirt:'#1f776f',shirt2:'#238b7e',shirt3:'#18575',pants:'#423b69',mouth:'#7a222c'};
 let lastPos='',lastChange=0,step=0;
 const stars=[[8,10],[22,30],[100,8],[90,26],[60,6],[110,40],[40,18]];
 function frame(now){
  const pos=document.getElementById('position-display')?.textContent||'';
  if(pos!==lastPos){lastPos=pos;lastChange=now;const m=pos.match(/(\d+)\s*:\s*(\d+)/);if(m)step=(Number(m[1])-1)*16+Number(m[2])-1;}
  const playing=now-lastChange<600,phase=step%4,head=playing?(phase===0?2:phase===1?1:0):0,walk=playing?(step>>1)&1:0,bob=playing?0:(Math.floor(now/700)&1);
  const meter=parseFloat(document.getElementById('meter')?.style.width)||0,mouth=Math.min(3,Math.floor(meter/22));
  const sel=document.querySelector('#tracks button[aria-selected=true]'),eye=playing&&sel?getComputedStyle(sel).getPropertyValue('--color').trim()||'#cbe094':'#cbe094';
  /* the night behind it */
  const g=ctx.createLinearGradient(0,0,0,120);g.addColorStop(0,'#0a1220');g.addColorStop(1,'#13241a');ctx.fillStyle=g;ctx.fillRect(0,0,120,120);
  stars.forEach(([x,y],i)=>{const tw=(Math.floor(now/(260+i*40))+i)%5;ctx.fillStyle=tw===0?'#fffbe0':tw<3?'#8ea087':'#4a5a4e';ctx.fillRect(x,y,tw===0?2:1,tw===0?2:1);});
  ctx.fillStyle='#e2e8ba33';ctx.beginPath();ctx.arc(98,20,12,0,7);ctx.fill();ctx.fillStyle='#e2e8ba';ctx.beginPath();ctx.arc(98,20,9,0,7);ctx.fill();ctx.fillStyle='#c4cc9e';ctx.beginPath();ctx.arc(95,18,2.4,0,7);ctx.fill();ctx.beginPath();ctx.arc(101,24,1.6,0,7);ctx.fill();
  ctx.fillStyle='#3f2e1e';ctx.fillRect(0,108,120,12);ctx.fillStyle='#54783a';ctx.fillRect(0,106,120,3);
  /* the zombie (the firmware's ui_zombie, at x = 12) */
  const x=12,y=1+bob,h=y+head;
  rect(x+9,y+29,22,16,C.dark);rect(x+11,y+29,18,14,C.shirt);rect(x+12,y+31,4,11,C.shirt2);rect(x+24,y+33,4,9,'#185754');
  if(playing&&walk){rect(x+2,y+16,8,17,C.shade);rect(x+2,y+14,8,4,C.skin);}else{rect(x+2,y+28+walk,8,17,C.shade);rect(x+2,y+40+walk,8,4,C.skin);}
  if(playing&&!walk){rect(x+30,y+16,8,17,C.skin);rect(x+30,y+14,8,4,C.light);}else{rect(x+30,y+28-walk,8,17,C.skin);rect(x+30,y+40-walk,8,4,C.light);}
  rect(x+11,y+43,8,9+walk,C.pants);rect(x+21,y+43,8,10-walk,C.pants);rect(x+10,y+51+walk,10,3,C.dark);rect(x+20,y+52-walk,10,3,C.dark);
  rect(x+5,h,30,29,C.dark);rect(x+7,h+2,26,25,C.skin);rect(x+7,h+2,26,5,C.shade);rect(x+9,h+3,5,3,C.light);rect(x+22,h+4,8,3,'#487f2e');
  rect(x+7,h+10,4,8,C.shade);rect(x+29,h+8,4,17,C.shade);rect(x+11,h+21,5,5,C.light);rect(x+22,h+22,6,4,C.shade);rect(x+17,h+15,5,6,C.shade);
  const blink=Math.floor(now/100)%45===0;rect(x+11,h+11,7,blink?2:5,C.dark);rect(x+23,h+11,7,blink?2:5,C.dark);
  if(!blink){rect(x+12,h+12,3,2,eye);rect(x+24,h+12,3,2,eye);}
  if(mouth){rect(x+15,h+22,10,2+mouth,C.dark);rect(x+17,h+23,6,mouth,C.mouth);}else{rect(x+15,h+23,10,2,C.dark);rect(x+19,h+25,7,1,C.shade);}
  setTimeout(()=>requestAnimationFrame(frame),66);
 }
 if(!matchMedia('(prefers-reduced-motion: reduce)').matches)requestAnimationFrame(frame);else frame(0);
}
