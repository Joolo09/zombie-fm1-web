/* SPDX-License-Identifier: MIT — explicit read-only host-report UI. */
import {HostLink,decode} from './protocol.js';
const $=id=>document.getElementById(id),link=new HostLink();let access,report,busy=false;
const hex=n=>'0x'+n.toString(16).toUpperCase().padStart(4,'0');
function message(text,error=false){$('message').textContent=text;$('message').classList.toggle('error',error);}
function render(){for(const id of ['ports','input','output'])$(id).disabled=busy||link.connected||id!=='ports'&&!access;
 $('connect').disabled=busy||link.connected||!$('input').value||!$('output').value;
 $('refresh').disabled=busy||!link.connected;$('disconnect').disabled=busy||!link.connected;$('export').disabled=!report||report.state!==8||!report.attempts;
 if(!report){$('result').textContent='Не прочитан';for(const id of ['ids','host','bytes','time'])$(id).textContent='—';$('raw').textContent='Нет данных';return;}$('result').textContent=(report.simulated?'СИМУЛЯЦИЯ · ':'')+report.resultText+' · '+report.signalingSpeed.toUpperCase()+' / FS: '+report.firstFullSpeed.resultText;
 $('ids').textContent=report.receivedBytes===18?hex(report.vid)+' / '+hex(report.pid):'Нет полного дескриптора';
 $('host').textContent=String(report.hostModeObserved)+' / '+String(report.resetAttempted);
 $('bytes').textContent=report.packets+' / '+report.receivedBytes;$('time').textContent=report.elapsedMs+' мс / '+report.lastStage;
 $('raw').textContent=JSON.stringify({registers:report.registers,descriptor:report.descriptor},null,2);
}
async function run(fn){if(busy)return;busy=true;render();try{await fn();}catch(e){message(e.message,true);}finally{busy=false;render();}}
$('ports').onclick=()=>run(async()=>{
 if(!navigator.requestMIDIAccess||!isSecureContext)throw Error('Нужен Chrome или Edge и HTTPS.');
 if(!access){access=await navigator.requestMIDIAccess({sysex:true});access.addEventListener('statechange',()=>{if(link.connected&&(link.input?.state==='disconnected'||link.output?.state==='disconnected')){link.close();message('USB отключён. После теста снова найди порты и подключись.');render();}});}
 for(const [id,ports] of [['input',access.inputs],['output',access.outputs]]){const select=$(id);select.replaceChildren();for(const p of ports.values())if(p.state!=='disconnected'&&/fm-?1|felucca|hortator/i.test(p.name||'')){const o=document.createElement('option');o.value=p.id;o.textContent=p.name;select.append(o);}}
 message($('input').value&&$('output').value?'Выбери порты FM-1 и нажми «Подключить и прочитать».':'Порты FM-1 не найдены. Переподключи USB и повтори поиск.');
});
$('connect').onclick=()=>run(async()=>{report=null;render();report=await link.connect(access.inputs.get($('input').value),access.outputs.get($('output').value));message(report.attempts?'Отчёт прочитан. Можно сохранить JSON.':'Host-тест ещё не запускался. Следуй инструкции выше.');});
$('refresh').onclick=()=>run(async()=>{report=decode(await link.command(5));message('Отчёт обновлён.');});
$('disconnect').onclick=()=>{link.close();message('Отключено. Отчёт остаётся доступен для скачивания.');render();};
$('export').onclick=()=>{if(!report||report.state!==8||!report.attempts)return;const blob=new Blob([JSON.stringify({...report,generatedAt:new Date().toISOString()},null,2)],{type:'application/json'}),url=URL.createObjectURL(blob),a=document.createElement('a');a.href=url;a.download='zombie-lab-force-report.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);};
window.addEventListener('pagehide',()=>link.close());render();
