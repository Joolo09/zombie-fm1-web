/* SPDX-License-Identifier: MIT — Zombie FM-1 web installer 2.0 (wizard over vendor/fm1ota.js). */
import {Updater,FM1_PORT} from './vendor/fm1ota.js?v=1.5';
import {checkPackage} from './package-check.js';

const $=id=>document.getElementById(id);
const text={
 ru:{navInstall:'Установка',navEmulator:'Эмулятор',navEditor:'Редактор',navNew:'Что нового',navHelp:'Помощь',
  headline:'Прошивка FM-1 прямо из браузера',lead:'Шесть дорожек, 96 пресетов, драм-машина, пианоролл. Установка займёт около минуты. Нужны кабель USB и Chrome или Edge.',
  ctaInstall:'Установить →',ctaTry:'Попробовать в эмуляторе',screenNote:'Экран снят в эмуляторе, который работает на том же C-коде, что и прошивка.',
  installEyebrow:'Веб-установщик',installTitle:'Установить в FM-1',
  chkBrowser:'Браузер с Web MIDI (Chrome, Edge, Opera на компьютере)',chkHttps:'Защищённое соединение (HTTPS)',chkPackage:'Файл прошивки скачан и проверен (SHA-256)',
  s1:'Выберите прошивку',archive:'Другие версии (архив)',s2:'Подключите FM-1',
  s2help:'Подключите FM-1 кабелем с передачей данных и включите его. Закройте FL Studio, Ableton и другие вкладки, которые используют MIDI. Браузер попросит разрешение на MIDI: нажмите «Разрешить».',
  noDevice:'Устройство не найдено',detect:'Найти FM-1',s3:'Установите',
  confirm:'Я сохранил свои проекты (экспорт JSON в редакторе) и WAV. Пока идёт запись, не отключаю USB и не закрываю страницу.',
  install:'Установить',s4:'Готово',s4help:'FM-1 перезапустится. Установщик дождётся устройства и проверит новую версию.',
  details:'Файл и журнал',download:'Скачать .fwsc',source:'Исходники',
  newTitle:'Что нового: число шагов',c1:'Удерживайте SEQ и крутите K1: число шагов дорожки от 1 до 64.',c2:'На драмке K1 задаёт одну длину сразу всем 16 звукам.',c3:'На экране «ШАГОВ 32», шаги после конца цикла затемнены, лишние банки приглушены.',c4:'LEN 16/32/64 в слое REC (REC + HOME) тоже меняют весь драм-кит.',c5:'Звук из 0.14.0 остался: STOP без щелчков, лимитер, ноты названиями.',c6:'0.15.1: исправлены 4 ошибки, найденные фаззингом и проверкой секвенсора (сбой экрана драмки, щелчки в FM, удвоение кита, импорт проекта).',
  notesLink:'Подробно: что изменено и как проверено',auditLink:'Отчёт тестов',ctrlTitle:'Главное управление',ctrlLink:'Полное управление драмкой',ctrlLink2:'Страницы и сочетания',
  helpTitle:'Если что-то пошло не так',moreTitle:'Ещё',licenses:'Лицензии',
  q1:'FM-1 не находится',q1a:'Нужен кабель с передачей данных: зарядный кабель не подойдёт.',q1b:'Закройте DAW и все вкладки редактора или эмулятора, подключённые к FM-1.',q1c:'Нажмите на значок замка в адресной строке, разрешите MIDI-устройства и обновите страницу.',q1d:'Оставьте подключённым только один FM-1.',
  q2:'USB отключился во время записи',q2a:'Не паникуйте: загрузчик FM-1 остаётся рабочим. Подключите кабель снова, выберите ту же версию, нажмите «Найти FM-1», затем «Установить». Запись начнётся заново, и результат проверится после перезапуска.',
  q3:'Установщик пишет «Open-прошивка»',q3a:'На FM-1 стоит официальная Open-прошивка M-VAVE, и она принимает только штатную FM-1 V15. Сначала верните её официальным установщиком M-VAVE, затем установите Zombie.',
  q4:'Звук трещит или новая версия не нравится',q4a:'Откройте «Другие версии» и установите 0.15.0 beta или стабильную 0.8.3. Перед откатом на 0.13.5 и старше экспортируйте проект в JSON в редакторе. 0.15.1 проверена в эмуляторе и тестах.',
  q5:'Как вернуть заводскую прошивку M-VAVE',q5a:'Воспользуйтесь официальным установщиком M-VAVE для FM-1 (V15). Сохранённые в Zombie проекты при этом не переносятся.',
  t1:'Эмулятор FM-1',t1a:'Экран, кнопки и звук на коде прошивки 0.15.1. Можно поиграть с клавиатуры ПК.',t2:'IDM-банк',t2a:'Послушайте 32 пресета до установки.',t3:'Профили FL Studio и Ableton',t3a:'Скрипты для режима DAW Controller, инструкции RU/EN.',
  k1:'<b>ALGO</b> выбирает дорожку, шестая — драмка. <b>SELECT</b> задаёт темп.',k2:'<b>SEQ</b>: 16 белых клавиш ставят и выбирают шаги. <b>SEQ + K1</b>: число шагов. <b>EDIT</b> удаляет ноту.',k3:'<b>PLAY</b> + <b>REC</b>: запись в ритм. Без PLAY запись идёт по шагам.',k4:'<b>EDIT + OCT−/OCT+</b>: отмена и повтор. <b>SAVE</b>: сохранить проект.',k5:'Драмка на SEQ: <b>OP1–OP6</b> выбирают звук, <b>PITCH/GLOBAL</b> листают группы.',k6:'<b>HOME + SEL</b>: Standalone → DAW Synth → DAW Controller.',
  stg_start:'подготовка',stg_verify:'проверка пакета',stg_loader:'загрузчик',stg_write:'запись',stg_reboot:'перезапуск',stg_done:'готово',tagrecommended:'рекомендуем',tagstable:'стабильная',tagbeta:'beta',tagarchive:'архив',archivePick:'— выбрать из архива —',
  checking:'Скачиваю и проверяю файл…',ready:'Файл проверен. Подключите FM-1 и нажмите «Найти FM-1».',finding:'Ищу FM-1 по USB MIDI…',
  found:'FM-1 найден. Отметьте подтверждение и нажмите «Установить».',same:'На FM-1 уже стоит эта версия. Её можно установить повторно.',
  start:'Подготовка устройства…',verify:'FM-1 проверяет пакет…',loader:'Перезапуск в загрузчик…',write:'Запись прошивки…',reboot:'Перезапуск и проверка версии…',
  done:'Готово! Прошивка установлена, FM-1 подтвердил версию. Проверьте звук и клавиши.',
  loadfail:'Файл не загрузился. Проверьте интернет и обновите страницу.',badpkg:'Проверка файла не прошла. В устройство ничего не отправлено. Обновите страницу.',
  nomidi:'Этот браузер не умеет Web MIDI. Откройте страницу в Chrome или Edge на компьютере (не на телефоне и не в Safari/Firefox).',nohttps:'Откройте страницу по HTTPS: без него браузер не даёт доступа к MIDI.',
  denied:'Доступ к MIDI запрещён. Нажмите на значок замка в адресной строке, разрешите MIDI-устройства и попробуйте снова.',
  notfound:'FM-1 не найден. Проверьте кабель (нужен кабель с данными) и закройте программы, которые используют MIDI.',multiple:'Подключено несколько FM-1. Оставьте один, чтобы было ясно, какой прошивать.',
  model:'Это устройство или загрузчик не FM-1. Запись не начата.',open:'На FM-1 официальная Open-прошивка M-VAVE: она не принимает Zombie напрямую. Сначала верните штатную FM-1 V15 официальным установщиком (см. «Помощь»).',
  lost:'USB отключился. Подключите FM-1 снова, выберите ту же версию и повторите установку: загрузчик примет запись заново.',stopped:'Передача остановилась. Подключите FM-1 снова и повторите установку той же версии.',
  noloader:'Загрузчик не появился. Переподключите USB и повторите установку той же версии.',noreturn:'FM-1 не вернулся после записи. Выключите и включите его, затем нажмите «Найти FM-1», чтобы проверить версию.',
  mismatch:'После записи FM-1 сообщил другую версию. Установка не подтверждена: повторите её.',appnotready:'USB-имя подтверждено, но приложение FM-1 не отвечает. Сохраните журнал, перезапустите FM-1 и повторите установку.',
  badreq:'Устройство запросило данные вне файла. Запись остановлена, повторите установку.',busy:'Установщик уже открыт в другой вкладке. Закройте её.',
  error:'Не получилось. Подробности в журнале ниже.',leave:'Идёт установка. Не закрывайте страницу.',beta:'Beta: проверена в эмуляторе и тестах, на настоящем FM-1 ещё нет. Если что-то не так, вернитесь на предыдущую версию.'},
 en:{navInstall:'Install',navEmulator:'Emulator',navEditor:'Editor',navNew:'What’s new',navHelp:'Help',
  headline:'Flash your FM-1 from the browser',lead:'Six tracks, 96 presets, a drum machine and a piano roll. Installing takes about a minute and needs a USB cable plus Chrome or Edge.',
  ctaInstall:'Install →',ctaTry:'Try the emulator',screenNote:'Screen captured in the emulator, which runs the same C code as the firmware.',
  installEyebrow:'Web installer',installTitle:'Install on FM-1',
  chkBrowser:'Browser with Web MIDI (Chrome, Edge, Opera on a computer)',chkHttps:'Secure connection (HTTPS)',chkPackage:'Firmware downloaded and verified (SHA-256)',
  s1:'Choose firmware',archive:'Other versions (archive)',s2:'Connect FM-1',
  s2help:'Connect FM-1 with a USB data cable and switch it on. Close FL Studio, Ableton and other tabs using MIDI. When the browser asks for MIDI access, click Allow.',
  noDevice:'No device found',detect:'Find FM-1',s3:'Install',
  confirm:'I have saved my projects (JSON export in the editor) and WAVs. I will not unplug USB or close this page while writing.',
  install:'Install',s4:'Done',s4help:'FM-1 restarts; the installer waits for it and verifies the new version.',
  details:'Package and log',download:'Download .fwsc',source:'Source',
  newTitle:'What\u2019s new: step count',c1:'Hold SEQ and turn K1: track step count from 1 to 64.',c2:'On the drum track K1 sets one length for all 16 sounds at once.',c3:'The screen shows STEPS 32, dims steps after the loop end and unused banks.',c4:'LEN 16/32/64 in the REC layer (REC + HOME) also set the whole drum kit.',c5:'The 0.14.0 sound stays: click-free STOP, limiter, note names.',c6:'0.15.1: 4 bugs found by fuzzing and a sequencer check are fixed (drum screen overflow, FM clicks, kit doubling, project import).',
  notesLink:'Details: changes and how they were tested',auditLink:'Test report',ctrlTitle:'Key controls',ctrlLink:'Full drum controls',ctrlLink2:'Pages and shortcuts',
  helpTitle:'Troubleshooting',moreTitle:'More',licenses:'Licenses',
  q1:'FM-1 is not found',q1a:'Use a USB data cable: a charge-only cable will not work.',q1b:'Close your DAW and any editor or emulator tabs connected to FM-1.',q1c:'Click the lock icon in the address bar, allow MIDI devices and reload.',q1d:'Keep only one FM-1 connected.',
  q2:'USB disconnected while writing',q2a:'Don’t panic: the FM-1 loader stays intact. Reconnect, choose the same version, click Find FM-1, then Install. Writing restarts and the result is verified after reboot.',
  q3:'The installer says “Open firmware”',q3a:'Your FM-1 runs M-VAVE’s official Open firmware, which only accepts stock FM-1 V15. Restore stock with M-VAVE’s official updater first, then install Zombie.',
  q4:'Crackling sound or you don’t like the new version',q4a:'Open “Other versions” and install 0.15.0 beta or the stable 0.8.3. Before rolling back to 0.13.5 or older, export your project to JSON in the editor. 0.15.1 was tested in the emulator and test suite.',
  q5:'Back to stock M-VAVE firmware',q5a:'Use M-VAVE’s official FM-1 updater (V15). Projects saved in Zombie are not carried over.',
  t1:'FM-1 emulator',t1a:'Screen, buttons and sound from the 0.15.1 firmware code. Playable from your PC keyboard.',t2:'IDM bank',t2a:'Listen to 32 presets before installing.',t3:'FL Studio & Ableton profiles',t3a:'Scripts for DAW Controller mode, RU/EN guide.',
  k1:'<b>ALGO</b> selects the track (6 = drums). <b>SELECT</b> sets tempo.',k2:'<b>SEQ</b>: 16 white keys place and select steps. <b>SEQ + K1</b>: step count. <b>EDIT</b> deletes a note.',k3:'<b>PLAY</b> + <b>REC</b>: live recording; without PLAY, step recording.',k4:'<b>EDIT + OCT−/OCT+</b>: undo / redo. <b>SAVE</b>: save the project.',k5:'Drums on SEQ: <b>OP1–OP6</b> choose the sound, <b>PITCH/GLOBAL</b> change group.',k6:'<b>HOME + SEL</b>: Standalone → DAW Synth → DAW Controller.',
  stg_start:'preparing',stg_verify:'checking package',stg_loader:'loader',stg_write:'writing',stg_reboot:'restarting',stg_done:'done',tagrecommended:'recommended',tagstable:'stable',tagbeta:'beta',tagarchive:'archive',archivePick:'— pick from the archive —',
  checking:'Downloading and checking the package…',ready:'Package verified. Connect FM-1 and click Find FM-1.',finding:'Looking for FM-1 over USB MIDI…',
  found:'FM-1 found. Tick the confirmation and click Install.',same:'FM-1 already runs this version. You can reinstall it.',
  start:'Preparing the device…',verify:'FM-1 is checking the package…',loader:'Restarting into the loader…',write:'Writing firmware…',reboot:'Restarting and verifying the version…',
  done:'Done! Firmware installed and version confirmed by FM-1. Check sound and keys.',
  loadfail:'Could not download the package. Check your connection and reload.',badpkg:'Package verification failed. Nothing was sent to the device. Reload the page.',
  nomidi:'This browser has no Web MIDI. Use Chrome or Edge on a computer (not a phone, Safari or Firefox).',nohttps:'Open this page over HTTPS; browsers only allow MIDI on secure pages.',
  denied:'MIDI access was blocked. Click the lock icon in the address bar, allow MIDI devices and try again.',
  notfound:'FM-1 not found. Check the cable (it must carry data) and close apps using MIDI.',multiple:'Several FM-1 are connected. Keep one so the target is unambiguous.',
  model:'This device or loader is not an FM-1. Writing did not start.',open:'FM-1 runs M-VAVE’s official Open firmware, which cannot take Zombie directly. Restore stock FM-1 V15 with the official updater first (see Help).',
  lost:'USB disconnected. Reconnect FM-1, choose the same version and install again; the loader accepts a fresh write.',stopped:'The transfer stopped. Reconnect FM-1 and install the same version again.',
  noloader:'The loader did not appear. Reconnect USB and install the same version again.',noreturn:'FM-1 did not come back after writing. Power-cycle it, then click Find FM-1 to check its version.',
  mismatch:'FM-1 reported a different version after writing. Not confirmed: install again.',appnotready:'USB identity confirmed, but the FM-1 application is not responding. Save the log, restart FM-1 and install again.',
  badreq:'The device asked for data outside the package. Writing stopped; install again.',busy:'The installer is open in another tab. Close it.',
  error:'Something went wrong. See the log below.',leave:'Installation in progress. Do not close this page.',beta:'Beta: tested in the emulator and test suite, not yet on a physical FM-1. Roll back if anything is off.'}
};
const HTML_KEYS=/^k\d$/;
let lang='ru';try{lang=localStorage.getItem('zcore-language')||(navigator.language||'ru').slice(0,2);}catch{}if(!text[lang])lang='ru';
const tr=k=>text[lang][k]??text.ru[k]??k;
let manifest,release,checked,access,detected,active=false,loading=false,generation=0,progress=0;
let message='',tone='',stage='',deviceText='',sameVersion=false;
const midiSupported=()=>!!navigator.requestMIDIAccess;
const secure=()=>window.isSecureContext&&!!crypto.subtle;

function render(){
 document.documentElement.lang=lang;$('lang').value=lang;
 document.querySelectorAll('[data-t]').forEach(e=>{const v=tr(e.dataset.t);if(HTML_KEYS.test(e.dataset.t))e.innerHTML=v;else e.textContent=v;});
 setCheck('chk-browser',midiSupported());setCheck('chk-https',secure());setCheck('chk-package',checked?true:loading||!release?null:false);
 renderReleases();
 $('release-note').textContent=release?(release.notes?.[lang]||'')+(release.channel==='recommended'||release.channel==='beta'?' '+tr('beta'):''):'';
 const status=$('status');status.textContent=message?tr(message):'';status.className='status'+(tone?' '+tone:'');
 $('device').textContent=deviceText||tr('noDevice');$('device').classList.toggle('connected',!!detected);
 const ready=!!checked&&midiSupported()&&secure();
 $('detect').disabled=active||loading||!ready;
 $('install-btn').disabled=active||loading||!checked||!detected||!$('confirm').checked;
 $('confirm').disabled=active;
 for(const el of document.querySelectorAll('#release-list input,#archive-select'))el.disabled=active||loading||!manifest;
 $('bar').style.width=progress+'%';$('pct').textContent=Math.round(progress)+'%';$('stage').textContent=stage?tr('stg_'+stage):'';
 const done=message==='done';
 setStep(1,checked?'done':'active');setStep(2,!checked?'locked':detected||done?'done':'active');setStep(3,!detected&&!done?'locked':done?'done':'active');setStep(4,done?'done':'locked');
 if(release){$('download').href=release.file;$('source').href=release.source||'downloads/groovebox-source-0.8.3.zip';$('editor-link').href=release.editor||'webapp/editor/';$('footer-version').textContent=release.label.en.split(' · ')[0].toUpperCase();}
 document.querySelector('.installer').setAttribute('aria-busy',String(active));
}
function setCheck(id,state){const el=$(id);el.classList.toggle('ok',state===true);el.classList.toggle('bad',state===false);}
function setStep(n,state){$('step-'+n).dataset.state=state;}
function say(key,t=''){message=key;tone=t;render();}
function log(line){$('log').textContent=(new Date().toLocaleTimeString()+' '+line+'\n'+$('log').textContent).slice(0,20000);}
function showError(e){const code=text.en[e?.code]?e.code:(e?.name==='NotAllowedError'||e?.name==='SecurityError'?'denied':'error');log((e?.code?e.code+': ':'')+(e?.message||String(e)));say(code,'bad');}
function clearDevice(){detected=null;deviceText='';sameVersion=false;}

/* Recommended and stable builds are cards; everything else lives in the archive select. */
let listBuilt=false;
function renderReleases(){
 if(!manifest)return;
 const featured=manifest.releases.filter(r=>r.channel==='recommended'||r.channel==='stable'||r.channel==='beta');
 const list=$('release-list'),select=$('archive-select');
 if(!listBuilt){
  list.textContent='';
  for(const r of featured){const label=document.createElement('label');label.className='release';label.innerHTML='<input type="radio" name="release"><strong><span class="name"></span><span class="tag"></span></strong><span class="note"></span>';
   const input=label.querySelector('input');input.value=r.product;input.addEventListener('change',()=>choose(r.product));label.querySelector('.tag').classList.add({recommended:'rec',stable:'stable',beta:'beta'}[r.channel]);list.append(label);}
  select.textContent='';const none=document.createElement('option');none.value='';select.append(none);
  for(const r of manifest.releases.filter(r=>!featured.includes(r)).sort((a,b)=>cmp(b.version,a.version))){const o=document.createElement('option');o.value=r.product;select.append(o);}
  listBuilt=true;
 }
 for(const label of list.children){const r=manifest.releases.find(x=>x.product===label.querySelector('input').value);label.querySelector('.name').textContent=r.label[lang];label.querySelector('.tag').textContent=tr('tag'+r.channel);label.querySelector('.note').textContent=r.version+' · '+(r.bytes/1024|0)+' KB';label.querySelector('input').checked=release?.product===r.product;}
 for(const o of select.options){if(!o.value){o.textContent=tr('archivePick');continue;}const r=manifest.releases.find(x=>x.product===o.value);o.textContent=r.label[lang];}
 select.value=release&&!featured.includes(release)?release.product:'';
 if(release&&!featured.includes(release))select.closest('details').open=true;
}
const cmp=(a,b)=>{const x=a.split('.').map(Number),y=b.split('.').map(Number);for(let i=0;i<3;i++)if(x[i]!==y[i])return x[i]-y[i];return 0;};

function choose(product){if(active||product===release?.product)return;const url=new URL(location.href);url.searchParams.set('release',product);history.replaceState(null,'',url);loadRelease(product);}
async function loadRelease(product){
 const token=++generation;checked=null;clearDevice();loading=true;progress=0;stage='';
 release=manifest.releases.find(r=>r.product===product)||manifest.releases.find(r=>r.product===manifest.current);say('checking');
 try{
  const response=await fetch(release.file,{cache:'no-store'});if(!response.ok)throw Object.assign(Error('Package HTTP '+response.status),{code:'loadfail'});
  const next=await checkPackage(new Uint8Array(await response.arrayBuffer()),release);if(token!==generation)return;
  checked=next;$('checksum').textContent='SHA-256 '+release.sha256;log('OK '+release.product+' '+release.version+' SHA-256 '+release.sha256.slice(0,16)+'…');
  say(!midiSupported()?'nomidi':!secure()?'nohttps':'ready',!midiSupported()||!secure()?'bad':'');
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
 if(active||!checked)return;active=true;clearDevice();say('finding');
 try{await withDeviceLock(async()=>{
  if(!access){access=await navigator.requestMIDIAccess({sysex:true});access.addEventListener('statechange',()=>{if(!active&&detected&&(!access.inputs.has(detected.inputId)||access.inputs.get(detected.inputId).state==='disconnected')){clearDevice();say('ready');}});}
  ensureSingle();const up=new Updater(access,log);const dev=await up.find();if(!dev)throw Object.assign(Error('No FM-1 response'),{code:'notfound'});
  try{
   if(!['FM-1','ota-FM-1'].includes(dev.id.model))throw Object.assign(Error(dev.id.text),{code:'model'});
   detected={inputId:dev.link.input.id};sameVersion=dev.id.text===checked.product;
   deviceText=dev.id.text+(dev.id.open?' · M-VAVE Open':'')+' · '+dev.name;log('DEVICE '+dev.id.text);
   say(sameVersion?'same':'found',dev.id.open?'warn':'ok');
  }finally{await dev.link.close();}
 });}catch(e){showError(e);}finally{active=false;render();}
}
async function install(){
 if(active||!checked||!detected||!$('confirm').checked)return;active=true;progress=0;stage='start';say('start');let wake,success=false;
 try{await withDeviceLock(async()=>{
  ensureSingle();try{wake=await navigator.wakeLock?.request('screen');}catch{}
  const up=new Updater(access,log);let last='';log('INSTALL '+checked.product);
  await up.install(checked.image,checked.product,(key,count,address)=>{
   if(key!==last){log(key+(typeof count==='string'?' '+count:''));last=key;}
   const fixed={start:3,verify:10,loader:30,reboot:97,done:100};
   const amount=key==='write'?35+Math.min(1,(address||0)/checked.image.length)*60:fixed[key];
   if(amount!=null)progress=Math.max(progress,amount);stage=key;say(key,key==='done'?'ok':'');
  });
  success=true;
 });}catch(e){showError(e);}finally{await wake?.release().catch(()=>{});active=false;if(success){progress=100;stage='done';deviceText=checked.product+' · FM-1';say('done','ok');}detected=success?detected:null;if(!success)deviceText='';render();}
}

$('lang').addEventListener('change',e=>{lang=e.target.value;try{localStorage.setItem('zcore-language',lang);}catch{}render();});
$('archive-select').addEventListener('change',e=>{if(e.target.value)choose(e.target.value);});
$('detect').addEventListener('click',detect);$('install-btn').addEventListener('click',install);$('confirm').addEventListener('change',render);
window.addEventListener('beforeunload',e=>{if(active){e.preventDefault();e.returnValue=tr('leave');}});
render();
try{
 const response=await fetch('releases.json',{cache:'no-store'});if(!response.ok)throw Object.assign(Error('Manifest HTTP '+response.status),{code:'loadfail'});
 manifest=await response.json();if(manifest.format!=='zombie-web/1'||!Array.isArray(manifest.releases)||!manifest.releases.length)throw Error('Invalid release manifest');
 for(const r of manifest.releases)r.channel||='archive';
 const requested=new URLSearchParams(location.search).get('release');
 await loadRelease(manifest.releases.some(r=>r.product===requested)?requested:manifest.current);
}catch(e){showError(e);}
