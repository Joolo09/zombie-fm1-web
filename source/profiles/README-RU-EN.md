# Zombie Groovebox / Sampler · DAW profiles 0.8.0

Два отдельных профиля для FL Studio и два для Ableton Live 11/12 (Python 3).
Выбирай профиль установленной прошивки: Groovebox или Sampler. Скрипты
не включают DAW-режим сами: при каждом запуске FM-1 остаётся синтезатором /
сэмплером с локальным звуком. HOME + SEL переключает режимы на текущий сеанс:
Standalone → DAW Synth / Local OFF → DAW Controller → Standalone.
Режим также доступен в GLO, страница 6, K1. Для возврата к звуку без тестового
тона выбери Standalone. HOME + PLAY по-прежнему включает тихий тест выхода.

## FL Studio

1. Распакуй нужную папку из `fl-studio` в пользовательскую папку FL:
   `Documents/Image-Line/FL Studio/Settings/Hardware/`.
   Если User data folder перенесена, используй её `Settings/Hardware`.
2. Перезапусти FL или обнови MIDI scripts. В F10 включи MIDI Input устройства,
   выбери `Zombie Groovebox FM-1` / `Zombie Sampler FM-1` в Controller type.
3. Назначь одинаковый свободный Port входу и выходу FM-1, например 20.
4. Для плагинов включи на FM-1 DAW Controller. Для звука FM-1 через MIDI Out
   выбери DAW Synth, MIDI Out Port 20, каналы 1–6. Для секвенсора FM-1 с его
   собственным звуком и MIDI-выходом оставь Standalone.

ALGO выбирает первые шесть каналов Channel Rack. SELECT — громкость, PRESET —
панорама. K1–K4 отправляют CC74/71/73/72, по умолчанию доступны для Link to
controller / Multilink. Для точного автоматического назначения укажи четыре
индекса параметров в `MACRO_PARAMETER_INDICES` внутри `device_*.py`; значение
`None` оставляет MIDI Learn. Нельзя считать первые четыре параметра любого
VST его макросами: индексы выбираются под конкретный плагин.
PLAY запускает/останавливает FL, REC управляет записью. Повтор одинаковой
команды не переключает транспорт второй раз. Ноты и Note Off остаются на
обычном MIDI-пути FL; скрипт их не дублирует.

Для внешнего Clock включи Send master sync у Output FM-1 и USB / TRS Clock
на FM-1. Аудио записывается с USB-аудиовхода FM-1 либо аналогового выхода
через аудиоинтерфейс; MIDI сам по себе аудио не передаёт.

## Ableton Live

1. Создай `Remote Scripts` в User Library и скопируй туда нужную папку:
   `Zombie_Groovebox_FM1` или `Zombie_Sampler_FM1` из `ableton`.
2. Перезапусти Live. Preferences → Link, Tempo & MIDI: выбери соответствующую
   Control Surface и вход/выход FM-1. Вход: Track + Remote; выход: Remote.
3. Выбери нужную дорожку и устройство / Instrument Rack. Включи Monitor In
   либо вооружи дорожку для записи нот. Скрипт не делает это без команды.
4. Для управления плагинами включи DAW Controller на FM-1. Для игры звуком
   FM-1 из Live — DAW Synth и External Instrument / External MIDI Out,
   канал 1–6; аудиовход выбери отдельно.

K1–K4 управляют макросами 1–4 выбранного Rack (индексы 1–4, индекс 0 — Device On).
Настрой `MACRO_PARAMETER_INDICES` в settings.py под обычный инструмент.
SELECT — громкость, PRESET — панорама выбранной дорожки. ALGO — первые
шесть MIDI/Audio tracks Live. PLAY — транспорт, REC — Arrangement Record;
для Session Record измени RECORD_TARGET на "session".
Для синхронизации секвенсора включи Sync у MIDI Output FM-1, USB / TRS Clock
на устройстве и выбери нужный режим транспорта.

## Пэды Sampler

Белые клавиши — 16 пэдов активного банка. Банк A: MIDI 36–51, банк B: 52–67.
Чёрные 1/2 меняют банк; следующие — reverse, mode, CHOP 4/8/16, PANIC.
В Live используй Drum Rack. В FL используй FPC или другой сэмплер и назначь
его пэдам ноты 36–67 через note learn: заводская раскладка FPC может отличаться.
Отпускание отправляет Note Off той же ноты и канала, даже после смены банка /
дорожки. ONE SHOT намеренно доигрывает сэмпл; GATE/LOOP останавливается с Release.

Для встроенного Sampler ручки HOME: K1 — пэд, K2 — банк, K3 — уровень,
K4 — источник. PRESET меняет страницы Trim/Tune и Mode/Reverse/Choke/Bits.
Настоящие USR WAV нужно загрузить через веб-редактор Sampler.

## Piano roll Groovebox

SEQ открывает пианоролл на FM-1. K1 — шаг, K2 — нота, K3 — velocity,
K4 — длительность в шагах (0 = AUTO). Повторное SEQ меняет страницу:
аккорд 1/2/3 + chance; затем банк / длина / запись / квантование.
PRESET меняет банк из 16 шагов, SELECT — BPM, ALGO — дорожку.
Удерживай SEQ + белую клавишу для переключения шага. EDIT + OCT−/OCT+
делает Undo/Redo. REC + PLAY записывает игру; REC не включает PLAY сам.
Стартовый грув запускается только кнопкой PLAY. При наличии сохранённого
проекта загружается он, поэтому заводской грув может быть заменён проектом.

В браузере: клик добавляет/меняет ноту, drag вправо задаёт длительность,
Shift+клик добавляет ноту аккорда, Alt+клик удаляет ноту. Стрелки / Space /
+ / − работают, когда пианоролл в фокусе. При активном REC ручное
редактирование блокируется.

## English quick start

Copy the matching FL folder to your FL User data folder's Settings/Hardware,
or the matching Ableton folder to User Library/Remote Scripts. Select the
matching controller and FM-1 Input/Output. In FL use the same free MIDI port
number for Input and Output. In Live enable Input Track/Remote and Output
Remote; enable Output Sync only when you want to send MIDI clock.
HOME + SEL cycles Standalone → DAW Synth → DAW Controller for this session.
Each boot restores Standalone. K1–K4: CC74/71/73/72; ALGO: tracks/channels 1–6;
SELECT: volume; PRESET: pan. PLAY/REC control the DAW. FL defaults to MIDI Learn;
optional explicit plugin parameter indices are in the device script. Live
defaults to Rack Macros 1–4, configurable in settings.py. Record/monitor the
track yourself; scripts do not arm tracks or echo notes automatically.
Sampler pad banks send notes 36–51 / 52–67. Map these in Drum Rack / FPC.
Use the matching web editor for WAV uploads and project backups. MIDI carries
performance data; record the FM-1 audio input separately.

## Validation and sources

The scripts use the public FL MIDI scripting API and Live's installed
_Framework. All four profiles pass isolated API contract tests for transport,
mapping, feedback, rate limiting and note passthrough. These are simulated
DAW APIs: actual FL/Live sessions and physical FM-1 are not tested here.

Application / scripts: MIT. Official installation/API references:
https://www.image-line.com/fl-studio-learning-content/fl-studio-online-manual/html/midi_scripting.htm
https://help.ableton.com/hc/en-us/articles/209072009-Installing-third-party-remote-scripts
