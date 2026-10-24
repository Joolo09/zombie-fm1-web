# Zombie Lab test website

Public entry: https://joolo09.github.io/zombie-fm1-web/lab/
Diagnostics: ./diagnostics/. Frozen firmware 0.1.0, FM-1_62000100.
USB DEVICE only. No USB host, disk support, unknown RAM probes or flash-data writes
through diagnostics. Installation and OTA do write the application/staging area.
No automatic flashing, MIDI requests or diagnostics on page load.
The separate installer manifest also offers return to Groovebox 0.8.2.
The browser shares the origin-wide installer lock with the main site.
Reports are downloaded locally, never uploaded. Original source/licensing in
downloads/Zombie-Lab-Source.zip. Hardware untested; browser and protocol checks
use fake devices. Main website files and all firmware binaries are unchanged.
