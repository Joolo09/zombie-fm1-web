/* SPDX-License-Identifier: MIT — decorative feedback, independent of audio and MIDI. */
const reduced=()=>globalThis.matchMedia?.('(prefers-reduced-motion: reduce)').matches;
const flashes=new WeakMap(),entries=new WeakMap();
export function animatePanel(panel){
 if(!panel||reduced()||typeof panel.animate!=='function')return;
 entries.get(panel)?.cancel();
 entries.set(panel,panel.animate([{opacity:.25,transform:'translateY(7px)'},{opacity:1,transform:'translateY(0)'}],{duration:210,easing:'cubic-bezier(.2,.7,.3,1)'}));
}
export function initInterface(){
 const flash=button=>{
  if(!button||button.disabled||button.closest('[inert]')||reduced()||typeof button.animate!=='function')return;
  flashes.get(button)?.cancel();
  flashes.set(button,button.animate([{filter:'brightness(1.3)'},{filter:'brightness(1)'}],{duration:230,easing:'ease-out'}));
 };
 document.addEventListener('pointerdown',event=>{if(event.button===0)flash(event.target.closest?.('button,.file-button'));},{passive:true});
 document.addEventListener('keydown',event=>{if(!event.repeat&&['Enter',' '].includes(event.key))flash(event.target.closest?.('button'));},{passive:true});
 document.body.classList.add('interface-ready');
}
