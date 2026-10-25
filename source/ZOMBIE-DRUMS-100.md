# Groovebox 0.10.0 beta: Zombie drum mechanics

This corrects the scope of 0.9.x. Track 6 uses Zombie's original synthesized GM kit, panel and shared clock/effects. The Hortator runtime, UI, project format and global controls are not linked. The five synth tracks retain their original audio algorithms and controls; tests compare all five tracks and eight engines against the released Zombie WASM byte for byte.

Track 6 contains 16 independent sound parts with 64 steps each, velocity, probability and rolls (1–4 hits). Each part has its own length (1–64), level, decay, tune, pan and mute. Closed and open hats choke one another. Drum voices do not take synth voices. PLAY/STOP/REC/BPM/swing/MIDI clock remain shared with Zombie. Pattern actions operate on the selected drum part; copying a drum part into a synth track is intentionally unavailable.

Device controls in SEQ on track 6:
- Page 1: K1 step, K2 sound, K3 velocity, K4 roll.
- Page 2: K1 step, K2 probability, K3 part length, K4 mute.
- Page 3: K1 tune, K2 decay, K3 level, K4 pan.
- Hold SEQ + white key: toggle that step for the selected sound. SEQ + OCT selects a bank. White keys without SEQ audition/record the kit.
- ALGO selects the track and SELECT sets BPM as before. EDIT + OCT handles undo/redo.

The browser editor is webapp/groovebox-100/. Drum controls are confined to track 6 in Zombie's existing editor. Project JSON includes a validated zombie-drums/1 section; five synth tracks remain in zombie-core/3. PC preview uses the firmware C engine.

Persistence: drum state is 3208 bytes in a separate A/B flash journal (0x97000/0x98000), committed only by SAVE. Original 0.8.x drum steps migrate from the GM kit. Hortator 0.9.x kit data is not imported into this different format; export needed 0.9.x kits in their old editor before saving this version. Existing five-track settings use their original journal. No physical flashing is performed by build or tests.

Build: Python source/build-groovebox-100.py (same AC79 SDK and Jieli toolchain requirements as the earlier source archive). Device, host and WASM builds are checked. Hardware behavior and real-time load still require a physical FM-1 test.

Checks:
- node tests/test_zombie_drums.mjs: synth equivalence, 16 parts, silence at zero probability, rolls, independent lengths, MIDI stop/continue, multi-pad recording, invalid import rejection, undo/redo and no h_ exports.
- Run host simulator against tests/zombie-drum-controls.txt: actual panel controls, flash save/reboot, BPM and stopped boot.
- Python tests/test_save_led.py HOST_EXECUTABLE: K2/SAVE regression.
- node tests/web-installer.mjs: installer/manifest checks.

Historical 0.9.x source remains only for earlier releases and is excluded from the 0.10.0 source archive and build. The default release remains 0.8.3 until the new behavior is approved and tested.
