# Zombie FM-1 · Groovebox / Sampler

Альтернативная прошивка для M-VAVE FM-1: шесть дорожек (пять синтезаторов и драм-кит), 96 пресетов, 64-шаговый секвенсор, пианоролл, арпеджиатор, режимы DAW. Отдельная прошивка Sampler: 32 пэда.

**Установка из браузера:** https://joolo09.github.io/zombie-fm1-web/. Нужны Chrome или Edge на компьютере и USB-кабель с передачей данных. Установщик сам никогда ничего не прошивает: запись начинается только по кнопке.

## Groovebox 0.15.0 beta: число шагов

- **SEQ + K1**: число шагов выбранной дорожки (1–64). На драмке одна общая длина сразу для всех 16 звуков.
- На экране «ШАГОВ 32», шаги после конца цикла затемнены, неиспользуемые банки приглушены.
- LEN 16/32/64 в слое REC меняют весь драм-кит.

[Подробности](source/GROOVEBOX-150-RU.md).

## Groovebox 0.14.0 beta: чистый звук

- STOP, выключение дорожки и перехват голосов больше не щёлкают: звук затухает примерно за 25 мс.
- Лимитер на шине вместо жёсткого клиппинга.
- Повтор одной ноты не занимает все 8 голосов.
- ATTACK 0 без щелчка.
- Ноты и аккорды на экране показываются названиями.
- Управление и формат проектов 0.13.7 не изменились.

[Подробности и проверки](source/GROOVEBOX-140-RU.md). Физический FM-1 не проверен. Стабильная 0.8.3 и все прежние версии доступны в установщике («Другие версии»).

## Сайт

- `index.html` и `installer.js`: пошаговый установщик. Он проверяет браузер, HTTPS и SHA-256 файла, предлагает рекомендуемую, стабильную и архивные версии, проверяет версию на устройстве после перезапуска и подсказывает, что делать при ошибках.
- `emulator/`: FM-1 в браузере на C-коде прошивки.
- `webapp/groovebox-150/`: редактор для 0.15.0. Редакторы прежних версий лежат в своих папках.
- `releases.json`: список прошивок. Поле `channel` принимает значения `recommended`, `stable`, `beta` или `archive`.

## Сборка и тесты

```
python -X utf8 tests/run_groovebox_audit.py --build
```

Скрипт собирает прошивку устройства, хост-симулятор, WASM и эмулятор, затем выполняет 35 проверок. Для сборки нужны тулчейн JieLi pi32v2 (`JIELI_TOOLCHAIN`), три файла AC79 SDK (`AC79_SDK`: `cpu/wl82/tools/uboot.boot`, `cfg_tool.bin`, `cfg/eq_cfg_hw.bin`; их SHA-256 записаны в `source/platform-081/tools/build.py`), `zig` 0.13 (`ZIG`) и `cc`. Готовый пакет: `python -X utf8 source/package-groovebox-150.py`.

Исходники: `source/editions/` и полный архив `downloads/groovebox-source-0.15.0.zip`. Приложение, DSP, интерфейс и профили распространяются по лицензии MIT, аппаратная платформа и обновление — по MPL-2.0. Звуки Minecraft в Sampler принадлежат Mojang/Microsoft, их происхождение описано в архиве исходников.

## История

Примечания к прежним выпускам лежат в `source/GROOVEBOX-1*-RU.md` и `downloads/*-RU.md`.

<details><summary>Прежний README</summary>

# Zombie FM-1 · Groovebox / Sampler

Groovebox 0.10.1 beta fixes on-device recording and editing: immediate REC, preserved fast taps/retriggers, musical step gates, live overwrite, quantized-hit suppression and explicit step/part/track erasure with undo. The obsolete Hortator source import is removed; old release archives remain available. [Device controls and migration](downloads/Groovebox-0.10.1-RU.md).

Groovebox 0.10.0 beta corrects the earlier Hortator integration: original Zombie drum mechanics live only on track 6, with the five synth tracks unchanged. See [controls, migration and tests](source/ZOMBIE-DRUMS-100.md). The default remains 0.8.3; older 0.9.x downloads retain their earlier behavior.

Groovebox 0.8.3 fixes the SAVE LED staying lit after K2 edits. Unsaved edits
use an on-screen `*`; SAVE still saves and pulses when pressed. Controls,
project format and DAW profiles remain compatible with 0.8.2. Sampler stays
0.8.0. [Release notes](downloads/Groovebox-0.8.3-RU.md).
The installer also checks the running application's status after its USB identity
returns, so an identity-only safe-mode response cannot report installation success.

Site / installer: https://joolo09.github.io/zombie-fm1-web/

Two dedicated firmware packages. Groovebox: six synthesis tracks, 64 original
presets, native/web four-note piano roll and starter groove. Sampler: 32 pads,
four user PCM slots, chopping, reverse, choke groups and one-shot/gate/loop.
Both boot Standalone with local sound; HOME + SEL cycles Standalone → DAW Synth
→ DAW Controller for the current session. Never automatically flashes a device.

Original 240x240 RGB565 device UI: track colors, output meter, page transitions,
fading pad hits, moving sequencer playhead. English and Russian labels.
Displayed previews are from the host simulator, not physical hardware photos.

Find FM-1, then Install in Chrome/Edge with MIDI SysEx. Back up projects/WAVs
and close the DAW / other MIDI tabs first. Firmware updates are validated with
size, SHA-256 and package identity; success requires the restarted identity.

Dedicated FL Studio and Ableton Live 11/12 Python 3 profiles are in
source/profiles/ and downloads/Zombie-FM1-DAW-Profiles-0.8.0.zip. Read the included
RU/EN guide for installation, pad note maps and manual MIDI Learn in FL.

Both firmware binaries compile; host, actual WASM audio and stub DAW API tests
pass. Physical FM-1 flashing/display and actual FL/Live sessions are not verified
by the developer. Older 0.7.4 / 0.7.1 downloads remain available.

Editable new firmware source: source/editions/. Complete matching platform,
generators/assets and profiles: downloads/editions-source-0.8.0.zip.
Application/DSP/UI/profiles: MIT. Hardware/boot/update platform: MPL-2.0.
Minecraft audio in Sampler: Mojang/Microsoft, provenance in source archive.
GitHub Pages: main, / (root), HTTPS. Paths support a repository subdirectory.


Groovebox 0.8.1: five synth tracks + synthesized GM drum kit on track six.
Native/web drum grid, 16 sounds, hat choke. Successful OTA now uses a full
P33 chip reset. Hardware confirmation pending. Details: downloads/Groovebox-0.8.1-RU.md.
Source: downloads/groovebox-source-0.8.1.zip. Sampler remains 0.8.0.


Groovebox 0.8.2: SELECT always BPM, remembered pages, 180ms holds, HOME layer
lock, SEQ bank shortcuts and additive kit step toggles. Original Zombie FM-1
240x240 splash restored. New matching DAW profiles map relative tempo CC116.
Details: downloads/Groovebox-0.8.2-RU.md. Physical hardware unverified.
Source: downloads/groovebox-source-0.8.2.zip. Sampler stays 0.8.0.


## Groovebox 0.9.0 beta: nested Hortator drums

Track 6 contains eight Hortator drum lanes with its actual DSP, native UI,
sequencer, modulation, effects, motion and song functions. Tracks 1–5 keep their
existing synth audio and panel. Physical selection: ALGO for the outer track,
SEL + PRESET for the inner drum. Experimental editor: `webapp/groovebox-090/`.

This beta has simulator/WASM/protocol checks; physical FM-1 timing, controls,
storage and updates remain unverified. 0.8.3 remains the default.
[Russian release notes](downloads/Groovebox-0.9.0-RU.md) ·
[features, adaptations, source/licensing and tests](source/drum-suite/README.md).
The combined 0.9.0 firmware/WASM is GPL-3.0-only; standalone original Zombie
source retains its MIT notices. The complete matching source archive is
`downloads/groovebox-source-0.9.0.zip`.

Groovebox 0.9.1 beta: [audit fixes and verification](source/AUDIT-091.md),
[Russian release notes](downloads/Groovebox-0.9.1-RU.md). Default remains 0.8.3.

</details>
