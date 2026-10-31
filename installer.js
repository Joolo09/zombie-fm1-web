/* SPDX-License-Identifier: MIT — Zombie FM-1 web installer 2.0 (wizard over vendor/fm1ota.js). */
import {Updater,FM1_PORT} from './vendor/fm1ota.js?v=1.6';
import {checkPackage} from './package-check.js';

const $=id=>document.getElementById(id);
const text={
 ru:{navInstall:'Установка',navEmulator:'Эмулятор',navEditor:'Редактор',navNew:'Что нового',navHelp:'Помощь',
  
  ctaInstall:'Установить →',navFeatures:"Функции",navSavers:"Заставки",headline:"Zombie Groovebox для FM-1",lead:"Шесть дорожек, 128 пресетов, драм-машина на 16 звуков, пианоролл до 64 шагов и экран, который живёт вместе с музыкой. Ставится из браузера за минуту.",screenNote:"Настоящие кадры прошивки 0.18.2: сняты в симуляторе на том же C-коде.",fEyebrow:"Что умеет",fTitle:"Целый трек — прямо на FM-1",f1t:"Звук: 6 дорожек и 128 пресетов",f1:"Пять синтезаторных дорожек и драмка на шестой. Восемь движков: аналог с расстройкой, FM, фазовый, wavetable, шум и ударные. Три банка: заводской, <b>IDM</b> и новый <b>ZMB</b> — 808, вобл-басы, синтвейв, хоррор-атмосферы. Фильтр, огибающая, LFO, drive, delay, reverb и арпеджиатор на каждой дорожке.",f2t:"Пианоролл и шаги",f2:"До <b>64 шагов</b> на дорожку (SEQ + K1). Зажми шаг и крути <b>K1–K4</b>: нота, сила, длина, гейт. Аккорды. <b>Лад и тоника</b> — клавиши не промахиваются мимо тональности. SEQ-инструменты: <b>Эвклид</b>, случайный рисунок, половина, <b>COOK</b>. Отмена и повтор на 8 шагов.",f3t:"Драм-машина на 16 звуков",f3:"Сетка на шестой дорожке: белые клавиши ставят удары, <b>OP1–OP6</b> выбирают звук. Зажми шаг и крути K2–K4: <b>шанс, сила, дробь</b>. У каждого звука своя длина петли — полиритмы без настроек. Готовый бит одной кнопкой.",f4t:"Живой экран",f4:"Зомби качает головой в такт, ноты разлетаются искрами цвета дорожки, ручки подсвечиваются при повороте. На страницах — картинка того, что меняют ручки: огибающая, волна, фильтр, микшер. <b>HOME дважды</b> — «О системе»: нагрузка, MIDI, время работы, когда включится заставка.",r1t:"Запись",r1:"<b>REC</b> с главной: отсчёт и живая запись по кругу с наложением. На SEQ — по шагам. Метроном и отсчёт 1–2 такта.",r2t:"MIDI и DAW",r2:"USB MIDI вход и выход, <b>MIDI clock</b> наружу. Режимы DAW Synth и DAW Controller, профили для FL Studio и Ableton.",r3t:"Веб-редактор",r3:"Все параметры, проекты в JSON, настройки и выбор заставки — с компьютера по USB, без установки программ.",sEyebrow:"Заставки",sTitle:"Минута без касаний — и экран оживает",fSaversLead:"Четыре заставки, все двигаются под музыку. Когда заставка на экране, <b>держи HOME</b> — включится следующая. Любая кнопка будит экран и срабатывает как обычно. Выбрать или выключить: <b>GLO → стр. 5 → K4</b>, <b>SAVE</b> запоминает.",sv1t:"Ночь",sv1:"Луна, летучие мыши, кладбище. Дорожки горят свечами.",sv2t:"Синтвейв",sv2:"Закат, неоновые горы, сетка едет в темп.",sv3t:"Матрица",sv3:"Цифровой дождь вспыхивает цветами дорожек.",sv4t:"Лого",sv4:"Табличка отскакивает от краёв. Попадёт в угол?",k7:"<b>HOME дважды</b>: «О системе». На заставке <b>держи HOME</b>: следующая заставка.",k8:"<b>GLO</b>: настройки — лад, отсчёт, MIDI clock, заставка. Ещё нажатие — следующая страница.",iEyebrow:"Установка",iTitle:"Поставить за минуту",iLead:"Нужны компьютер с Chrome или Edge и USB-кабель с передачей данных. Установщик сам проверит файл, а после перезапуска — версию.",i1:"Сохраните проекты: экспорт JSON в веб-редакторе.",i2:"Подключите FM-1 и нажмите «Найти FM-1».",i3:"Нажмите «Установить» и не закрывайте вкладку до конца.",ctaTry:'Попробовать в эмуляторе',
  installEyebrow:'Веб-установщик',installTitle:'Установить в FM-1',
  chkBrowser:'Браузер с Web MIDI (Chrome, Edge, Opera на компьютере)',chkHttps:'Защищённое соединение (HTTPS)',chkPackage:'Файл прошивки скачан и проверен (SHA-256)',
  s1:'Выберите прошивку',archive:'Другие версии (архив)',s2:'Подключите FM-1',
  s2help:'Подключите FM-1 кабелем с передачей данных и включите его. Закройте FL Studio, Ableton и другие вкладки, которые используют MIDI. Браузер попросит разрешение на MIDI: нажмите «Разрешить».',
  noDevice:'Устройство не найдено',detect:'Найти FM-1',s3:'Установите',
  confirm:'Я сохранил свои проекты (экспорт JSON в редакторе) и WAV. Пока идёт запись, не отключаю USB и не закрываю страницу.',
  install:'Установить',s4:'Готово',s4help:'FM-1 перезапустится. Установщик дождётся устройства и проверит новую версию.',
  details:'Файл и журнал',download:'Скачать .fwsc',source:'Исходники',
  newTitle:'Что нового в 0.18.1: 4 заставки и «О системе»',c1:'Банк ZMB: 32 новых пресета (слоты 97–128): 808, вобл-басы, ривз, суперсо, синтвейв, хоррор-атмосферы.',c2:'Зомби на главной качает головой в такт, открывает рот под звук, машет руками; ноты разлетаются цветными искрами.',c3:'Ночная заставка: через минуту без касаний — луна, летучие мыши, кладбище; во время игры — темп и шесть «свечей» дорожек.',c4:'При включении зомби вылезает из земли. Карточки ручек вспыхивают при повороте, за курсором шагов бежит след.',c5:'0.18.1: четыре заставки (ночь, синтвейв, матрица, лого): на заставке держи HOME, чтобы сменить. HOME дважды — живой экран «О системе». Исправлено: на FM-1 заставка не появлялась.',c6:'Звук, проекты и управление 0.17 не изменились. Попробуйте в эмуляторе до установки.',
  notesLink:'Подробно: что изменено и как проверено',auditLink:'Отчёт тестов',ctrlTitle:'Главное управление',ctrlLink:'Полное управление драмкой',ctrlLink2:'Страницы и сочетания',
  helpTitle:'Если что-то пошло не так',moreTitle:'Ещё',licenses:'Лицензии',
  q1:'FM-1 не находится',q1a:'Нужен кабель с передачей данных: зарядный кабель не подойдёт.',q1b:'Закройте DAW и все вкладки редактора или эмулятора, подключённые к FM-1.',q1c:'Нажмите на значок замка в адресной строке, разрешите MIDI-устройства и обновите страницу.',q1d:'Оставьте подключённым только один FM-1.',
  q2:'USB отключился во время записи',q2a:'Не паникуйте: загрузчик FM-1 остаётся рабочим. Подключите кабель снова, выберите ту же версию, нажмите «Найти FM-1», затем «Установить». Запись начнётся заново, и результат проверится после перезапуска.',
  q3:'Установщик пишет «Open-прошивка»',q3a:'На FM-1 стоит официальная Open-прошивка M-VAVE, и она принимает только штатную FM-1 V15. Сначала верните её официальным установщиком M-VAVE, затем установите Zombie.',
  q4:'Звук трещит или новая версия не нравится',q4a:'Перед установкой сохраните проект в JSON в веб-редакторе. Если что-то звучит не так, напишите нам на GitHub. Вернуть заводскую прошивку можно официальным установщиком M-VAVE (пункт ниже).',
  q5:'Как вернуть заводскую прошивку M-VAVE',q5a:'Воспользуйтесь официальным установщиком M-VAVE для FM-1 (V15). Сохранённые в Zombie проекты при этом не переносятся.',
  t1:'Эмулятор FM-1',t1a:'Экран, кнопки и звук на коде прошивки 0.18.2. Можно поиграть с клавиатуры ПК.',t2:'Банки IDM и ZMB',t2a:'Послушайте 64 пресета до установки: IDM и новый банк ZMB 0.18.',t3:'Профили FL Studio и Ableton',t3a:'Скрипты для режима DAW Controller, инструкции RU/EN.',
  k1:'<b>ALGO</b> выбирает дорожку, шестая — драмка. <b>SELECT</b> задаёт темп.',k2:'<b>SEQ</b>: 16 белых клавиш ставят шаги; зажми шаг и крути ручки. <b>SEQ + K1</b>: число шагов. <b>EDIT</b> удаляет ноту.',k3:'<b>REC</b> с главной: отсчёт и живая запись по кругу. На <b>SEQ</b> — по шагам. <b>REC + HOME</b>: слой записи.',k4:'<b>EDIT + OCT−/OCT+</b>: отмена и повтор. <b>SAVE</b>: сохранить проект.',k5:'Драмка на SEQ: <b>OP1–OP6</b> выбирают звук, <b>PITCH/GLOBAL</b> листают группы.',k6:'<b>HOME + SEL</b>: Standalone → DAW Synth → DAW Controller.',
  stall:'FM-1 не запрашивает данные уже 15 с. Не отключайте USB и не закрывайте вкладку: часто запись продолжается сама. Если ничего не изменится за 3 минуты, установщик предложит повторить.',stg_stall:'ожидание FM-1',resuming:'Связь с FM-1 прервалась. Установщик ждёт загрузчик и сам продолжит запись. Если USB отключился, подключите его снова. Не выключайте FM-1 и не закрывайте страницу.',stg_resume:'продолжение записи',check:'FM-1 проверяет весь пакет перед записью: пока ничего не стирается. Потом начнётся запись.',stg_check:'проверка в FM-1',stg_start:'подготовка',stg_verify:'проверка пакета',stg_loader:'загрузчик',stg_write:'запись',stg_reboot:'перезапуск',stg_done:'готово',tagrecommended:'рекомендуем',tagstable:'стабильная',tagbeta:'beta',tagarchive:'архив',archivePick:'— выбрать из архива —',
  checking:'Скачиваю и проверяю файл…',ready:'Файл проверен. Подключите FM-1 и нажмите «Найти FM-1».',finding:'Ищу FM-1 по USB MIDI…',
  found:'FM-1 найден. Отметьте подтверждение и нажмите «Установить».',same:'На FM-1 уже стоит эта версия. Её можно установить повторно.',
  start:'Подготовка устройства…',verify:'FM-1 проверяет пакет…',loader:'Перезапуск в загрузчик…',write:'Запись прошивки во флеш FM-1: обычно 1–2 минуты. Счётчик блоков внизу показывает, что запись идёт. Не отключайте USB.',reboot:'Перезапуск и проверка версии…',
  done:'Готово! Прошивка установлена, FM-1 подтвердил версию. Проверьте звук и клавиши.',
  loadfail:'Файл не загрузился. Проверьте интернет и обновите страницу.',badpkg:'Проверка файла не прошла. В устройство ничего не отправлено. Обновите страницу.',
  nomidi:'Этот браузер не умеет Web MIDI. Откройте страницу в Chrome или Edge на компьютере (не на телефоне и не в Safari/Firefox).',nohttps:'Откройте страницу по HTTPS: без него браузер не даёт доступа к MIDI.',
  denied:'Доступ к MIDI запрещён. Нажмите на значок замка в адресной строке, разрешите MIDI-устройства и попробуйте снова.',
  notfound:'FM-1 не найден. Проверьте кабель (нужен кабель с данными) и закройте программы, которые используют MIDI.',multiple:'Подключено несколько FM-1. Оставьте один, чтобы было ясно, какой прошивать.',
  model:'Это устройство или загрузчик не FM-1. Запись не начата.',open:'На FM-1 официальная Open-прошивка M-VAVE: она не принимает Zombie напрямую. Сначала верните штатную FM-1 V15 официальным установщиком (см. «Помощь»).',
  lost:'USB отключился во время записи. Чёрный экран FM-1 сейчас — это режим загрузчика, он ждёт продолжения. Подключите USB снова, обновите страницу, выберите ту же версию, нажмите «Найти FM-1» и «Установить»: запись продолжится.',stopped:'Передача остановилась. Чёрный экран FM-1 — это режим загрузчика, ничего не сломано. Не выключайте FM-1: обновите страницу, выберите ту же версию, нажмите «Найти FM-1» и «Установить» — запись начнётся заново.',
  noloader:'Загрузчик не появился. Переподключите USB и повторите установку той же версии.',noreturn:'FM-1 не вернулся после записи. Выключите и включите его, затем нажмите «Найти FM-1», чтобы проверить версию.',
  mismatch:'После записи FM-1 сообщил другую версию. Установка не подтверждена: повторите её.',appnotready:'USB-имя подтверждено, но приложение FM-1 не отвечает. Сохраните журнал, перезапустите FM-1 и повторите установку.',
  badreq:'Устройство запросило данные вне файла. Запись остановлена, повторите установку.',busy:'Установщик уже открыт в другой вкладке. Закройте её.',
  error:'Не получилось. Подробности в журнале ниже.',leave:'Идёт установка. Не закрывайте страницу.',beta:'Проверена в эмуляторе и автотестах. Перед установкой сохраните проект в JSON.'},
 en:{navInstall:'Install',navEmulator:'Emulator',navEditor:'Editor',navNew:'What’s new',navHelp:'Help',
  
  ctaInstall:'Install →',navFeatures:"Features",navSavers:"Screensavers",headline:"Zombie Groovebox for FM-1",lead:"Six tracks, 128 presets, a 16-sound drum machine, a piano roll up to 64 steps and a screen that moves with the music. Installs from your browser in a minute.",screenNote:"Real 0.18.2 firmware frames, captured in the simulator running the same C code.",fEyebrow:"What it does",fTitle:"A whole track, right on the FM-1",f1t:"Sound: 6 tracks, 128 presets",f1:"Five synth tracks plus drums on the sixth. Eight engines: detuned analog, FM, phase, wavetable, noise and percussion. Three banks: factory, <b>IDM</b> and the new <b>ZMB</b> — 808s, wobble basses, synthwave, horror atmospheres. Filter, envelope, LFO, drive, delay, reverb and an arpeggiator on every track.",f2t:"Piano roll and steps",f2:"Up to <b>64 steps</b> per track (SEQ + K1). Hold a step and turn <b>K1–K4</b>: note, velocity, length, gate. Chords. <b>Scale and root</b> keep every key in key. SEQ tools: <b>Euclid</b>, random, half, <b>COOK</b>. Eight-step undo and redo.",f3t:"16-sound drum machine",f3:"A grid on track six: white keys place hits, <b>OP1–OP6</b> pick the sound. Hold a step and turn K2–K4: <b>chance, velocity, roll</b>. Each sound has its own loop length — polyrhythms for free. A ready beat with one press.",f4t:"A living screen",f4:"The zombie headbangs to the beat, notes fly off as sparks in their track colour, knobs light up as you turn them. Each page draws what its knobs change: envelope, wave, filter, mixer. <b>HOME twice</b> — About: load, MIDI, uptime, when the screensaver comes.",r1t:"Recording",r1:"<b>REC</b> from home: a count-in, then a live overdub loop. On SEQ: step by step. Click and a 1–2 bar count-in.",r2t:"MIDI and DAW",r2:"USB MIDI in and out, <b>MIDI clock</b> out. DAW Synth and DAW Controller modes, FL Studio and Ableton profiles.",r3t:"Web editor",r3:"Every parameter, JSON projects, settings and the screensaver choice — from your computer over USB, nothing to install.",sEyebrow:"Screensavers",sTitle:"A minute untouched, and the screen comes alive",fSaversLead:"Four screensavers, all moving with the music. While one is on screen, <b>hold HOME</b> for the next. Any button wakes the screen and works as usual. Pick or switch off: <b>GLO → page 5 → K4</b>, <b>SAVE</b> keeps it.",sv1t:"Night",sv1:"Moon, bats, graveyard. Tracks burn as candles.",sv2t:"Synthwave",sv2:"Sunset, neon hills, a grid running in tempo.",sv3t:"Matrix",sv3:"Digital rain flashing in track colours.",sv4t:"Bounce",sv4:"A badge bouncing off the edges. Will it hit the corner?",k7:"<b>HOME twice</b>: About. On a screensaver, <b>hold HOME</b>: the next one.",k8:"<b>GLO</b>: settings — scale, count-in, MIDI clock, screensaver. Press again for the next page.",iEyebrow:"Install",iTitle:"Installed in a minute",iLead:"You need a computer with Chrome or Edge and a USB data cable. The installer checks the package, then the version after the restart.",i1:"Save your projects: JSON export in the web editor.",i2:"Connect FM-1 and click Find FM-1.",i3:"Click Install and keep this tab open until it finishes.",ctaTry:'Try the emulator',
  installEyebrow:'Web installer',installTitle:'Install on FM-1',
  chkBrowser:'Browser with Web MIDI (Chrome, Edge, Opera on a computer)',chkHttps:'Secure connection (HTTPS)',chkPackage:'Firmware downloaded and verified (SHA-256)',
  s1:'Choose firmware',archive:'Other versions (archive)',s2:'Connect FM-1',
  s2help:'Connect FM-1 with a USB data cable and switch it on. Close FL Studio, Ableton and other tabs using MIDI. When the browser asks for MIDI access, click Allow.',
  noDevice:'No device found',detect:'Find FM-1',s3:'Install',
  confirm:'I have saved my projects (JSON export in the editor) and WAVs. I will not unplug USB or close this page while writing.',
  install:'Install',s4:'Done',s4help:'FM-1 restarts; the installer waits for it and verifies the new version.',
  details:'Package and log',download:'Download .fwsc',source:'Source',
  newTitle:'What\u2019s new in 0.18.1: 4 screensavers and an About screen',c1:'ZMB bank: 32 new presets (slots 97-128): 808s, wobble basses, reese, supersaw, synthwave, horror atmospheres.',c2:'The home-screen zombie headbangs to the beat, opens its mouth with the sound and waves; notes fly off as coloured sparks.',c3:'Night screensaver after a minute untouched: moon, bats, graveyard; while playing it shows the tempo and six track candles.',c4:'At power-on the zombie climbs out of the ground. Knob cards flash as you turn them; the step cursor leaves a trail.',c5:'0.18.1: four screensavers (night, synthwave, matrix, bounce): hold HOME on one to switch. HOME twice: a live About screen. Fixed: the screensaver never came up on FM-1.',c6:'Sound, projects and controls of 0.17 are unchanged. Try it in the emulator before installing.',
  notesLink:'Details: changes and how they were tested',auditLink:'Test report',ctrlTitle:'Key controls',ctrlLink:'Full drum controls',ctrlLink2:'Pages and shortcuts',
  helpTitle:'Troubleshooting',moreTitle:'More',licenses:'Licenses',
  q1:'FM-1 is not found',q1a:'Use a USB data cable: a charge-only cable will not work.',q1b:'Close your DAW and any editor or emulator tabs connected to FM-1.',q1c:'Click the lock icon in the address bar, allow MIDI devices and reload.',q1d:'Keep only one FM-1 connected.',
  q2:'USB disconnected while writing',q2a:'Don’t panic: the FM-1 loader stays intact. Reconnect, choose the same version, click Find FM-1, then Install. Writing restarts and the result is verified after reboot.',
  q3:'The installer says “Open firmware”',q3a:'Your FM-1 runs M-VAVE’s official Open firmware, which only accepts stock FM-1 V15. Restore stock with M-VAVE’s official updater first, then install Zombie.',
  q4:'Crackling sound or you don’t like the new version',q4a:'Save your project to JSON in the web editor before installing. If something sounds wrong, tell us on GitHub. The stock firmware comes back with M-VAVE’s official updater (below).',
  q5:'Back to stock M-VAVE firmware',q5a:'Use M-VAVE’s official FM-1 updater (V15). Projects saved in Zombie are not carried over.',
  t1:'FM-1 emulator',t1a:'Screen, buttons and sound from the 0.18.2 firmware code. Playable from your PC keyboard.',t2:'IDM and ZMB banks',t2a:'Listen to 64 presets before installing: IDM and the new ZMB bank in 0.18.',t3:'FL Studio & Ableton profiles',t3a:'Scripts for DAW Controller mode, RU/EN guide.',
  k1:'<b>ALGO</b> selects the track (6 = drums). <b>SELECT</b> sets tempo.',k2:'<b>SEQ</b>: 16 white keys place steps; hold a step and turn the knobs. <b>SEQ + K1</b>: step count. <b>EDIT</b> deletes a note.',k3:'<b>REC</b> from home: count-in and a live loop. On <b>SEQ</b>: step entry. <b>REC + HOME</b>: the REC layer.',k4:'<b>EDIT + OCT−/OCT+</b>: undo / redo. <b>SAVE</b>: save the project.',k5:'Drums on SEQ: <b>OP1–OP6</b> choose the sound, <b>PITCH/GLOBAL</b> change group.',k6:'<b>HOME + SEL</b>: Standalone → DAW Synth → DAW Controller.',
  stall:'FM-1 has not asked for data for 15 s. Keep USB connected and this tab open: writing often resumes by itself. If nothing changes within 3 minutes the installer will offer to retry.',stg_stall:'waiting for FM-1',resuming:'The link to FM-1 dropped. The installer is waiting for the loader and will resume writing by itself. If USB came out, plug it back in. Keep FM-1 on and this page open.',stg_resume:'resuming the write',check:'FM-1 checks the whole package before writing; nothing is erased yet. Writing starts next.',stg_check:'checking in FM-1',stg_start:'preparing',stg_verify:'checking package',stg_loader:'loader',stg_write:'writing',stg_reboot:'restarting',stg_done:'done',tagrecommended:'recommended',tagstable:'stable',tagbeta:'beta',tagarchive:'archive',archivePick:'— pick from the archive —',
  checking:'Downloading and checking the package…',ready:'Package verified. Connect FM-1 and click Find FM-1.',finding:'Looking for FM-1 over USB MIDI…',
  found:'FM-1 found. Tick the confirmation and click Install.',same:'FM-1 already runs this version. You can reinstall it.',
  start:'Preparing the device…',verify:'FM-1 is checking the package…',loader:'Restarting into the loader…',write:'Writing the firmware into FM-1 flash: usually 1–2 minutes. The block counter below shows it is moving. Keep USB connected.',reboot:'Restarting and verifying the version…',
  done:'Done! Firmware installed and version confirmed by FM-1. Check sound and keys.',
  loadfail:'Could not download the package. Check your connection and reload.',badpkg:'Package verification failed. Nothing was sent to the device. Reload the page.',
  nomidi:'This browser has no Web MIDI. Use Chrome or Edge on a computer (not a phone, Safari or Firefox).',nohttps:'Open this page over HTTPS; browsers only allow MIDI on secure pages.',
  denied:'MIDI access was blocked. Click the lock icon in the address bar, allow MIDI devices and try again.',
  notfound:'FM-1 not found. Check the cable (it must carry data) and close apps using MIDI.',multiple:'Several FM-1 are connected. Keep one so the target is unambiguous.',
  model:'This device or loader is not an FM-1. Writing did not start.',open:'FM-1 runs M-VAVE’s official Open firmware, which cannot take Zombie directly. Restore stock FM-1 V15 with the official updater first (see Help).',
  lost:'USB disconnected while writing. A black FM-1 screen now is the update loader waiting to continue. Reconnect, reload this page, choose the same version, click Find FM-1, then Install: writing resumes.',stopped:'The transfer stopped. A black FM-1 screen is the update loader; nothing is broken. Keep FM-1 on: reload this page, choose the same version, click Find FM-1, then Install to write again.',
  noloader:'The loader did not appear. Reconnect USB and install the same version again.',noreturn:'FM-1 did not come back after writing. Power-cycle it, then click Find FM-1 to check its version.',
  mismatch:'FM-1 reported a different version after writing. Not confirmed: install again.',appnotready:'USB identity confirmed, but the FM-1 application is not responding. Save the log, restart FM-1 and install again.',
  badreq:'The device asked for data outside the package. Writing stopped; install again.',busy:'The installer is open in another tab. Close it.',
  error:'Something went wrong. See the log below.',leave:'Installation in progress. Do not close this page.',beta:'Tested in the emulator and the test suite. Save your project to JSON before installing.'}
};
const HTML_KEYS=/^(k\d|f\d|r\d|fSaversLead)$/;
let lang='ru';try{lang=localStorage.getItem('zcore-language')||(navigator.language||'ru').slice(0,2);}catch{}if(!text[lang])lang='ru';
const tr=k=>text[lang][k]??text.ru[k]??k;
let manifest,release,checked,access,detected,active=false,loading=false,generation=0,progress=0;
let message='',tone='',stage='',stageNote=0,deviceText='',sameVersion=false;
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
 $('bar').style.width=progress+'%';$('pct').textContent=Math.round(progress)+'%';$('stage').textContent=stage?tr('stg_'+stage)+(stageNote?' · '+stageNote:''):'';
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
  document.querySelector('details.archive').hidden=select.options.length<=1;
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
const RESUMABLE=new Set(['lost','stopped','noloader']);
async function install(){
 if(active||!checked||!detected||!$('confirm').checked)return;active=true;progress=0;stage='start';say('start');let wake,success=false;
 try{await withDeviceLock(async()=>{
  ensureSingle();try{wake=await navigator.wakeLock?.request('screen');}catch{}
  const up=new Updater(access,log);let last='',loaderReached=false,pass=1,far=0;log('INSTALL '+checked.product);
  /* The loader reads flash.bin twice: pass 1 checks the whole package (nothing erased), pass 2 restarts at the app
   * area and erases + writes sector by sector. Progress follows the pass, so the real write never sits at 94 %. */
  const FL_END=0x400+0x93000,APP_START=0x400+0x4000;
  const report=(key,count,address)=>{
   if(key!==last){log(key+(typeof count==='string'?' '+count:''));last=key;}
   if(key==='loader'||key==='write')loaderReached=true;
   if(key==='loader'){pass=1;far=0;}
   if(key==='write'){address=address||0;if(pass===1&&far>APP_START+0x10000&&address<far-0x10000){pass=2;log('pass 2: erase + write');}far=Math.max(far,address);}
   const shown=key==='write'&&pass===1?'check':key;
   const fixed={start:3,verify:10,loader:30,reboot:97,done:100};
   const amount=key!=='write'?fixed[key]:pass===1?35+Math.min(1,address/FL_END)*25:60+Math.min(1,Math.max(0,address-APP_START)/(FL_END-APP_START))*35;
   if(amount!=null)progress=Math.max(progress,amount);stage=shown;stageNote=key==='write'?count:0;say(shown,key==='done'?'ok':key==='stall'?'warn':'');
  };
  /* Once the loader runs, FM-1 stays in update mode until a write completes (black screen). A cut USB link or a
   * stalled transfer is resumed here, so the page never leaves FM-1 waiting in the loader. */
  try{await up.install(checked.image,checked.product,report);}
  catch(e){
   if(!loaderReached||!RESUMABLE.has(e?.code))throw e;
   let error=e,resumed=false;
   for(let attempt=1;attempt<=3&&!resumed;attempt++){
    log('AUTO-RESUME '+attempt+' after '+error.code);progress=35;pass=1;far=0;stage='resume';say('resuming','warn');
    const ota=await up.waitFor(id=>/^ota-/i.test(id.model),90000);if(!ota)break;await ota.link.close();
    try{ensureSingle();await up.resume(checked.image,checked.product,report);resumed=true;}catch(again){error=again;if(!RESUMABLE.has(again?.code))throw again;}
   }
   if(!resumed)throw error;
  }
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
