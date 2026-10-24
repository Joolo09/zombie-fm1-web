/* SPDX-License-Identifier: MIT — original browser installer UI. */
import {Updater, FM1_PORT} from './vendor/fm1ota.js?v=lab2';
import {checkPackage} from './package-check.js';


const $=id=>document.getElementById(id);
const text={
 ru:{editor:'Открыть редактор ↗',headline:'Твой FM-1. Твой звук.',intro:'Редактируй звук и устанавливай Zombie Core прямо из браузера. По обычному USB-кабелю.',tryEditor:'Открыть студию →',preview:'Для знакомства устройство не нужно: синтезатор работает и в браузере.',installTitle:'Установить в FM-1',browserHelp:'Открой в Chrome или Edge на компьютере. Подключи FM-1 кабелем с передачей данных и закрой FL Studio, Ableton и другие вкладки редактора.',releaseLabel:'Версия прошивки',stepPackage:'Проверка файла',stepDevice:'Подключение FM-1',stepWrite:'Установка',stepDone:'Перезапуск',noDevice:'Устройство не подключено',detect:'Найти FM-1',install:'Установить',progress:'Ход установки',notice:'Установка заменит прошивку FM-1. Сначала сохрани проекты и исходные WAV. Во время записи не отключай USB и не закрывай страницу.',details:'Файл и журнал',download:'Скачать .fwsc',resumeTitle:'Если USB оборвался',resumeHelp:'Подключи FM-1 снова, выбери ту же версию и нажми «Найти FM-1», затем «Установить». Если загрузчик доступен, запись начнётся заново, а результат проверится после перезапуска.',localTitle:'Звук остаётся у тебя',privacy:'Сэмплы и настройки передаются по USB прямо между браузером и FM-1. Для прослушивания без устройства открой студию и включи звук ПК.',source:'Исходники прошивки и лицензии',licenses:'Лицензии и авторство',checking:'Проверяю файл и SHA-256…',ready:'Файл проверен. Подключи FM-1 и нажми «Найти FM-1».',finding:'Ищу FM-1 по USB MIDI…',found:'FM-1 найден. Можно устанавливать.',start:'Подготовка устройства…',verify:'FM-1 проверяет пакет…',loader:'Ожидание загрузчика FM-1…',write:'Запись прошивки…',reboot:'Перезапуск и проверка версии…',done:'Прошивка установлена, версия подтверждена. Проверь звук и клавиши.',loadfail:'Файл не загрузился. Проверь соединение и обнови страницу.',badpkg:'Проверка прошивки не прошла. Ничего не отправлено в устройство. Обнови страницу.',nomidi:'Для USB MIDI открой эту страницу по HTTPS в Chrome или Edge на компьютере.',denied:'Разреши MIDI с SysEx в настройках сайта и попробуй снова.',notfound:'FM-1 не найден. Проверь USB-кабель и закрой программы, использующие MIDI.',multiple:'Оставь подключённым один FM-1, чтобы выбрать устройство для установки однозначно.',model:'Это устройство или загрузчик не для FM-1. Запись не начата.',open:'Эта официальная Open-прошивка не принимает Zombie Core напрямую. Сначала восстанови штатную FM-1 V15 официальным установщиком.',lost:'USB отключён. Подключи FM-1 снова, выбери ту же версию и повтори установку.',stopped:'Передача остановилась. Подключи FM-1 снова и повтори установку той же версии.',noloader:'Загрузчик не появился. Переподключи USB и повтори установку той же версии.',noreturn:'Устройство не вернулось после записи. Перезапусти FM-1 и проверь версию.',mismatch:'После записи FM-1 сообщил другую версию. Установка не подтверждена.',busy:'Установка уже открыта в другой вкладке. Дождись её завершения.',error:'Не удалось выполнить операцию. Подробности в журнале.',leave:'Идёт установка. Не закрывай страницу.'},
 en:{editor:'Open editor ↗',headline:'Your FM-1. Your sound.',intro:'Edit sounds and install Zombie Core directly from your browser. With a standard USB cable.',tryEditor:'Open studio →',preview:'No device needed to explore: the synthesizer also runs in your browser.',installTitle:'Install on FM-1',browserHelp:'Use Chrome or Edge on a computer. Connect FM-1 with a USB data cable and close FL Studio, Ableton and other editor tabs.',releaseLabel:'Firmware release',stepPackage:'Check package',stepDevice:'Connect FM-1',stepWrite:'Install',stepDone:'Restart',noDevice:'No device connected',detect:'Find FM-1',install:'Install',progress:'Installation progress',notice:'Installing replaces the FM-1 firmware. Export projects and keep your original WAV files first. Do not disconnect USB or close this page while writing.',details:'Package and log',download:'Download .fwsc',resumeTitle:'If USB disconnects',resumeHelp:'Reconnect FM-1, select the same release and click Find FM-1, then Install. If the loader is available, the transfer restarts and the result is checked after reboot.',localTitle:'Your sounds stay with you',privacy:'Samples and settings travel directly between your browser and FM-1 over USB. To preview without a device, open the studio and enable PC audio.',source:'Firmware source and licenses',licenses:'Licenses and credits',checking:'Checking package and SHA-256…',ready:'Package verified. Connect FM-1 and click Find FM-1.',finding:'Looking for FM-1 over USB MIDI…',found:'FM-1 found. Ready to install.',start:'Preparing the device…',verify:'FM-1 is checking the package…',loader:'Waiting for the FM-1 loader…',write:'Writing firmware…',reboot:'Restarting and verifying the firmware version…',done:'Firmware installed and version confirmed. Check sound and keys.',loadfail:'Could not load the package. Check your connection and reload this page.',badpkg:'Firmware validation failed. Nothing was sent to the device. Reload this page.',nomidi:'For USB MIDI open this page over HTTPS in Chrome or Edge on a computer.',denied:'Allow MIDI with SysEx in the site settings and try again.',notfound:'FM-1 not found. Check the USB data cable and close apps using MIDI.',multiple:'Keep only one FM-1 connected so the installation target is unambiguous.',model:'This device or loader is not for FM-1. Writing did not start.',open:'This official Open firmware cannot install Zombie Core directly. Restore stock FM-1 V15 using the official updater first.',lost:'USB disconnected. Reconnect FM-1, choose the same release and try again.',stopped:'Transfer stopped. Reconnect FM-1 and install the same release again.',noloader:'The loader did not appear. Reconnect USB and install the same release again.',noreturn:'The device did not return after writing. Restart FM-1 and check its version.',mismatch:'FM-1 reported a different version after writing. Installation is not confirmed.',busy:'Another tab is already using the installer. Wait for it to finish.',error:'Operation failed. See the log for details.',leave:'Installation is in progress. Do not close this page.'}
};
Object.assign(text.ru,{"headline": "USB под наблюдением.", "intro": "Отдельный тестовый сайт Zombie Lab для FM-1. Установи диагностику, проверь USB и память, сохрани отчёт.", "installTitle": "Установить тестовую Lab", "releaseLabel": "Прошивка", "browserHelp": "Chrome или Edge на компьютере, USB-кабель с передачей данных. Перед установкой закрой DAW и другие MIDI-вкладки.", "notice": "Lab временно заменит музыкальную прошивку FM-1. Сохрани проекты и исходные WAV. Во время записи не отключай USB.", "done": "Пакет установлен, версия подтверждена. Открой выбранный режим.", "source": "Исходники Lab и лицензии", "preview": "На устройстве будет диагностика, без синтезатора. USB-host и диски эта версия не проверяет."});
Object.assign(text.en,{"headline": "USB under observation.", "intro": "A separate Zombie Lab test site for FM-1. Install diagnostics, check USB and memory, save a report.", "installTitle": "Install test Lab", "releaseLabel": "Firmware", "browserHelp": "Desktop Chrome or Edge, with a USB data cable. Close your DAW and other MIDI tabs before installation.", "notice": "Lab temporarily replaces the music firmware on FM-1. Export projects and keep source WAV files. Do not disconnect USB while writing.", "done": "Package installed and version confirmed. Open the selected mode.", "source": "Lab source and licenses", "preview": "The device will run diagnostics without synthesis. This version does not test USB host or drives."});
let lang='ru';try{lang=localStorage.getItem('zlab-language')||'ru';}catch{}if(!text[lang])lang='ru';
let manifest, release, checked, access, detected, active=false, loading=false, generation=0;
let message='checking', failure='', deviceText='', phase='package';
const supported=()=>window.isSecureContext && !!navigator.requestMIDIAccess && !!crypto.subtle;
const tr=k=>text[lang][k]||text[lang].error;
function render(){
 document.documentElement.lang=lang;$('language').value=lang;
 document.querySelectorAll('[data-t]').forEach(e=>e.textContent=tr(e.dataset.t));
 $('status').textContent=failure?'':tr(message);$('error').hidden=!failure;$('error').textContent=failure?tr(failure):'';
 $('device').textContent=deviceText||tr('noDevice');$('device').classList.toggle('connected',!!detected);
 $('detect').disabled=active||loading||!checked||!supported();$('install').disabled=active||loading||!checked||!detected;
 $('release').disabled=active||loading||!manifest;$('release-note').textContent=release?.notes?.[lang]||'';
 if(release){$('package-badge').textContent=release.version;if(release.product!=='FM-1_62000100'){$('install-heading').textContent=lang==='ru'?'Вернуть Groovebox 0.8.2':'Restore Groovebox 0.8.2';$('install').textContent=lang==='ru'?'Вернуть Groovebox':'Restore Groovebox';document.querySelector('[data-t=notice]').textContent=lang==='ru'?'Будет установлена музыкальная прошивка Groovebox 0.8.2. Сначала скачай отчёт Lab. Не отключай USB во время записи.':'Groovebox 0.8.2 music firmware will be installed. Download your Lab report first. Keep USB connected while writing.';}for(const a of document.querySelectorAll('[data-release-editor]')){a.href=release.editor;a.textContent=release.product==='FM-1_62000100'?(lang==='ru'?'Открыть диагностику →':'Open diagnostics →'):(lang==='ru'?'Открыть Groovebox →':'Open Groovebox →');}}
 for(const option of $('release').options){const item=manifest?.releases.find(r=>r.product===option.value);if(item)option.textContent=item.label[lang];}
 const stages=['package','device','write','done'];stages.forEach((s,i)=>{const el=$('step-'+s);el.classList.toggle('complete',i<stages.indexOf(phase)||message==='done');if(s===phase)el.setAttribute('aria-current','step');else el.removeAttribute('aria-current');});
 document.querySelector('.install-card').setAttribute('aria-busy',String(active));
}
function say(key){message=key;failure='';render();}
function log(line){$('log').textContent=(new Date().toLocaleTimeString()+' '+line+'\n'+$('log').textContent).slice(0,10000);}
function showError(e){failure=text.en[e?.code]?e.code:(e?.name==='NotAllowedError'||e?.name==='SecurityError'?'denied':'error');log(e?.message||String(e));render();}
function clearDevice(){detected=null;deviceText='';}
async function loadRelease(){
 const token=++generation;checked=null;clearDevice();loading=true;phase='package';$('progress').value=0;say('checking');
 try{
  release=manifest.releases.find(r=>r.product===$('release').value);if(!release)throw Error('Unknown release');render();
  const response=await fetch(release.file,{cache:'no-store'});if(!response.ok)throw Object.assign(Error('Package HTTP '+response.status),{code:'loadfail'});
  const next=await checkPackage(new Uint8Array(await response.arrayBuffer()),release);if(token!==generation)return;
  checked=next;$('checksum').textContent='SHA-256 '+release.sha256;$('firmware-download').href=release.file;
  phase='device';say(supported()?'ready':'nomidi');log('SHA-256 OK / '+release.product);
 }catch(e){if(token===generation)showError(e);}finally{if(token===generation){loading=false;render();}}
}
async function withDeviceLock(fn){
 if(!navigator.locks)return fn();
 return navigator.locks.request('zombie-fm1-installer',{ifAvailable:true},lock=>{if(!lock)throw Object.assign(Error('Installer busy'),{code:'busy'});return fn();});
}
function ensureSingle(){
 const candidates=[...access.inputs.values()].filter(p=>p.state!=='disconnected'&&FM1_PORT.test(p.name||''));
 if(candidates.length>1)throw Object.assign(Error('Multiple update candidates'),{code:'multiple'});
}
async function detect(){
 if(active||!checked||!supported())return;active=true;clearDevice();say('finding');
 try{await withDeviceLock(async()=>{
  if(!access){access=await navigator.requestMIDIAccess({sysex:true});access.addEventListener('statechange',()=>{if(!active&&detected&&(!access.inputs.has(detected.inputId)||access.inputs.get(detected.inputId).state==='disconnected')){clearDevice();say('ready');}});}
  ensureSingle();const up=new Updater(access,log);const dev=await up.find();if(!dev)throw Object.assign(Error('No FM-1 response'),{code:'notfound'});
  try{if(!['FM-1','ota-FM-1'].includes(dev.id.model))throw Object.assign(Error(dev.id.text),{code:'model'});detected={inputId:dev.link.input.id};deviceText=dev.id.text+' / '+dev.name;log('DEVICE '+dev.id.text);say('found');}finally{await dev.link.close();}
 });}catch(e){showError(e);}finally{active=false;render();}
}
async function install(){
 if(active||!checked||!detected)return;active=true;$('progress').value=0;say('start');let wake;
 try{await withDeviceLock(async()=>{
  ensureSingle();try{wake=await navigator.wakeLock?.request('screen');}catch{}
  const up=new Updater(access,log);let lastPhase='';
  await up.install(checked.image,checked.product,(key,count,address)=>{
   if(key!==lastPhase){log(key+(typeof count==='string'?' '+count:''));lastPhase=key;}
   phase=key==='done'||key==='reboot'?'done':'write';
   const fixed={start:3,verify:10,loader:30,reboot:97,done:100};
   const amount=key==='write'?35+Math.min(1,(address||0)/checked.image.length)*60:fixed[key];
   if(amount!=null)$('progress').value=Math.max($('progress').value,amount);
   say(key);
  });
 });}catch(e){showError(e);}finally{await wake?.release().catch(()=>{});active=false;clearDevice();render();}
}
$('language').addEventListener('change',e=>{lang=e.target.value;try{localStorage.setItem('zlab-language',lang);}catch{}render();});
$('release').addEventListener('change',loadRelease);$('detect').addEventListener('click',detect);$('install').addEventListener('click',install);
window.addEventListener('beforeunload',e=>{if(active){e.preventDefault();e.returnValue=tr('leave');}});
render();
try{
 const response=await fetch('releases.json',{cache:'no-store'});if(!response.ok)throw Object.assign(Error('Manifest HTTP '+response.status),{code:'loadfail'});
 manifest=await response.json();if(manifest.format!=='zombie-web/1'||!Array.isArray(manifest.releases)||!manifest.releases.length)throw Error('Invalid release manifest');
 for(const item of manifest.releases){const option=document.createElement('option');option.value=item.product;$('release').append(option);}
 $('release').value=manifest.current;await loadRelease();
}catch(e){showError(e);}
