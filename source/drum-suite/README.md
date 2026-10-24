# Hortator inside Zombie Groovebox track 6 (0.9.0 beta)

This combines the actual Hortator 0.23.3 drum DSP, sequencer and pixel UI with
Zombie's five existing synth tracks. It is an experimental device build:
no physical FM-1 was flashed or measured during development. Version 0.8.3
remains the default release.

## Controls

Select outer track 6 with ALGO. Its panel now shows Hortator.
On physical FM-1, ALGO still selects the outer track; hold SEL and turn PRESET
to select one of the eight inner drums. All other drum buttons/encoders retain
Hortator's roles, including SELECT for pages and the native held-button actions.
Turn ALGO back to tracks 1–5 to use the original synth panel.
The web editor at `webapp/groovebox-090/` has the drum screen, fourteen buttons,
seven encoders and all 27 keys. Here ALGO selects the inner drum.

## Included drum functionality

- 45 actual synthesis models; 151 factory sound presets and 128 user slots.
- Eight drum lanes, 64 steps each; per-lane length, division and swing.
- Step chance/conditions, accent, four-hit ratchets, transpose, ACID tie/slide.
- Live/step recording, track mutes, Grids map and Euclidean patterns.
- Sample model and layer, two LFOs and four modulation destinations per lane.
- Filter, distortion, slicer, resonators, chorus, delay and room/spring reverb.
- Sidechain compressor with ghost source; native perform effects.
- 128 motion events; four drum project slots; 16-row song chains.
- USB/TRS MIDI input, clock and MIDI learn through the existing platform.
- Separate `.zhkit` project import/export and browser flash persistence.

Device-global hardware setup, update/recovery, Bluetooth, USB audio, language,
master output and DAW modes remain the Zombie platform's responsibility. The
plain upstream reference does not include BLE. Its sample editor is a placeholder;
this integration does not promise a user-sample uploader. The legacy Groovebox
PCM-upload protocol is rejected, instead of copying into the smaller project buffer.

## Necessary adaptations and compatibility

The two complete runtimes exceed FM-1 RAM without changes. The original five
synth engines and their effects retain their audio implementation. Changes are
confined to the nested drum module and shared buffer allocation:

- Share the canvas, with a 240×124 strip (original synth graphs fit).
- Use a 4 KiB project-transfer buffer in Groovebox; Sampler keeps 48 KiB.
- Compand only Hortator's 65,536-sample delay ring to 8 bits. Maximum delay
  remains approximately 1.49 seconds, with reduced tail precision/noise quality.
- Reduce only offline waveform-preview extrema to signed 7-bit display values.
- Store exact signed-byte track parameters in **ZHD1**: 3,288 bytes/project.
  Native upstream FDRE/FDR/FUN project files are not compatible. Five-track
  Zombie projects keep their existing v3 format. Drum projects use `.zhkit`.
- Three reserved user-sample areas are 64 KiB each rather than upstream 80 KiB.
- The SAMPLE/layer built-in **ZOMBIE** GM kit is newly synthesized CC0 material,
  not the upstream Hügelton Sample Pack (whose asset terms are separate).
  Recreate it using `python source/drum-suite/gen_samples.py`.
- Native synthesis caps from Hortator remain: two ACID lanes, four resonators,
  at most two CHORD and two MODAL resonators. Remote edits/imports enforce them.
- Cosmetic LED dimming is sampled through Zombie's panel loop; its master glow
  setting remains in charge. Upstream USB/battery diagnostic indicators do not
  represent the combined platform's hardware status; use Zombie settings.

Memory report is recorded in the device build log. The native compiler also
reports a heuristic note for inlined 64-bit multiplies near loads in the original
fixed-point DSP. That note is not a hardware test. Combined worst-case DSP timing,
audio underruns, physical controls, flash power-loss behavior, MIDI hardware and
boot/update behavior still require testing on an actual FM-1.

## Flash layout

All offsets are within the platform's data region. Each object has two 4 KiB
sectors, header committed last with CRC; existing synth/calibration sectors
are explicitly outside the bridge write allowlist.

| Object | Sector pair starts |
| --- | --- |
| Drum settings | 0x97000 |
| Drum projects A/B/C/D | 0x99000 / 0x9B000 / 0x9D000 / 0xD2000 |
| MIDI learn | 0xD4000 |
| User preset bank | 0xD6000 |
| Reserved sample areas | 0xA0000 / 0xB0000 / 0xC0000 (64 KiB each) |
| Preserved Zombie sampler/synth/system/calibration/BLE | 0xD0000 / 0xD8000 / 0xDA000 / 0xDC000 / 0xDE000 |

## Protocol additions

Standard Zombie framing/pack7 applies. Commands 42–48 exist only in Groovebox 0.9+.

| Command | Payload | Action |
| --- | --- | --- |
| 42 | kind,u8 index,i16 value | Native button/key/encoder/hit (kinds 0/1/2/3) |
| 43 | empty | Snapshot the 3,288-byte drum project |
| 44 | lane,param,i16 value | Bounded lane parameter |
| 45 | lane,step,on,accent,condition,ratchet,i8 transpose | Step edit |
| 46 | global,i16 value | Bounded global parameter |
| 47 | CRC32 little endian | Begin drum-project upload |
| 17 | u16 offset, up to 192 data bytes | Upload ordered chunks |
| 48 | empty | Validate size, CRC and internal project; apply |

A pending upload blocks snapshots. Invalid projects leave the current drum state
untouched. Commands do not modify the first five synth tracks. Global transport
and tempo are shared, as in the original groovebox.

## Build and checks

Use the existing platform/toolchain setup, then:

```
python -X utf8 source/build-groovebox-090.py
node tests/test_hortator_suite.mjs
node tests/test_hortator_worklet.mjs
python -X utf8 tests/test_hortator_host.py source/editions/groovebox/build/host/zombie-groovebox_host
python -X utf8 tests/test_save_led.py source/editions/groovebox/build/host/zombie-groovebox_host
node --test tests/web-installer.mjs
```

Generated tables and font are committed for reproducibility. Their GPL generator
scripts and font source are under `source/third_party/hortator/`.

## Provenance and licensing

Official source: https://deadactive.github.io/hortator/source.tar.gz?v=b1815a759f
Version file: 0.23.3. Archive SHA-256:
`bda09b365189ef93f7258c390e4228af7ac7d0fe2f71ed426346b5f5f5d043f3`.
Hortator by DEADACTIVE, based on Felucca by Leo Kuroshita / Hügelton Instruments.
Unmodified upstream sources and notices are retained under `third_party/hortator`;
modified copies are under `drum-suite/ported`. The combined firmware and WASM
are distributed under GPL-3.0-only, with original MIT/MPL notices retained.
The original standalone Zombie synth source remains MIT. Read the upstream
LICENSE, LICENSING.md and LICENSES for individual component attribution.
The proprietary Hügelton sample assets and panel photograph are not included.
