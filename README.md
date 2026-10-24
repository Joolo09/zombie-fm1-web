# Zombie FM-1 · Groovebox / Sampler 0.8.0

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
