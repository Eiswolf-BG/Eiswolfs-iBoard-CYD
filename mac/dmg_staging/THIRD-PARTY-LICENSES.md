# Third-party components

Eiswolfs iBoard itself is distributed under the MIT License, see
[LICENSE](LICENSE). It builds on a number of open-source
components, each under its own license, listed here with the full
license text for each kept in [`licenses/`](licenses/).

This file covers all three parts of the project (CYD firmware, Mac
app, Windows listener) and the web installer page. The Mac app's DMG,
the Windows installer, and the web installer page each link back to
this file (and bundle a copy of `licenses/` where practical) rather
than duplicating it.

## CYD firmware (`cyd/`)

| Component | License | Source |
|---|---|---|
| ESP32 Arduino core (`framework = arduino`) | LGPL-2.1 | [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32) — see [`licenses/esp32-arduino-core-NOTICE.txt`](licenses/esp32-arduino-core-NOTICE.txt) |
| TFT_eSPI | FreeBSD License (compatible with GPL) | [Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) — [`licenses/tft_espi-FreeBSD.txt`](licenses/tft_espi-FreeBSD.txt) |
| XPT2046_Touchscreen | MIT | [PaulStoffregen/XPT2046_Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen) — [`licenses/xpt2046-MIT.txt`](licenses/xpt2046-MIT.txt) |
| ArduinoJson | MIT | [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson) — [`licenses/arduinojson-MIT.txt`](licenses/arduinojson-MIT.txt) |
| fauxmoESP (Alexa/Philips-Hue emulation, v1.2.0+; only started when switched on in the menu) | MIT (Copyright Xose Pérez) | [vintlabs/fauxmoESP](https://github.com/vintlabs/fauxmoESP) — [`licenses/fauxmoesp-MIT.txt`](licenses/fauxmoesp-MIT.txt) |
| AsyncTCP (pulled in by fauxmoESP, statically linked into the firmware) | **LGPL-3.0** | [ESP32Async/AsyncTCP](https://github.com/ESP32Async/AsyncTCP) — [`licenses/asynctcp-LGPL-3.0.txt`](licenses/asynctcp-LGPL-3.0.txt) |
| ESP-IDF (the SDK inside the ESP32 Arduino core) | Apache-2.0 | [espressif/esp-idf](https://github.com/espressif/esp-idf) — [`licenses/APACHE-2.0.txt`](licenses/APACHE-2.0.txt) |
| "Pixel" font (Press Start 2P) | SIL Open Font License 1.1 | Google Fonts `ofl/pressstart2p` — [`cyd/src/fonts/PressStart2P_OFL.txt`](cyd/src/fonts/PressStart2P_OFL.txt), see also [`cyd/fonts/LICENSES.md`](cyd/fonts/LICENSES.md) |
| "Klar" font (FreeSansBold, via TFT_eSPI/Adafruit-GFX) | BSD | [`cyd/fonts/freesansbold_bsd_license.txt`](cyd/fonts/freesansbold_bsd_license.txt), see also [`cyd/fonts/LICENSES.md`](cyd/fonts/LICENSES.md) |
| Umlaut-capable "Klar" font (`cyd/src/fonts/ui_bold_font.h`, generated from Liberation Sans Bold) | SIL Open Font License 1.1 | [Liberation Fonts](https://github.com/liberationfonts/liberation-fonts) — [`licenses/LIBERATION-FONTS-LICENSE.txt`](licenses/LIBERATION-FONTS-LICENSE.txt); generated with [`cyd/tools/generate_ui_font.py`](cyd/tools/generate_ui_font.py) (an earlier DejaVu Sans Bold-based attempt was replaced — DejaVu's glyphs are proportionally too wide for the CYD's narrow dashboard tiles) |

## Mac app (`mac/`)

| Component | License | Source |
|---|---|---|
| macmon (embedded binary) | MIT | [vladkens/macmon](https://github.com/vladkens/macmon) — [`licenses/macmon-MIT.txt`](licenses/macmon-MIT.txt) |
| m1ddc (embedded binary, optional monitor-input switching over DDC/CI) | MIT | [waydabber/m1ddc](https://github.com/waydabber/m1ddc) — [`licenses/m1ddc-MIT.txt`](licenses/m1ddc-MIT.txt) |
| rumps | BSD | [jaredks/rumps](https://github.com/jaredks/rumps) — [`licenses/rumps-BSD.txt`](licenses/rumps-BSD.txt) |
| psutil | BSD-3-Clause | [giampaolo/psutil](https://github.com/giampaolo/psutil) — [`licenses/psutil-BSD-3-Clause.txt`](licenses/psutil-BSD-3-Clause.txt) |
| pyserial | BSD | [pyserial/pyserial](https://github.com/pyserial/pyserial) — [`licenses/pyserial-BSD-NOTICE.txt`](licenses/pyserial-BSD-NOTICE.txt) (exact text not bundled locally, see notice) |
| pyobjc (AppKit/Foundation, used for sleep-event detection) | MIT | [ronaldoussoren/pyobjc](https://github.com/ronaldoussoren/pyobjc) — [`licenses/pyobjc-MIT.txt`](licenses/pyobjc-MIT.txt) |
| Python runtime + standard library (bundled by PyInstaller) | PSF License 2.0 | [python/cpython](https://github.com/python/cpython) — [`licenses/python-PSF-NOTICE.txt`](licenses/python-PSF-NOTICE.txt) |
| PyInstaller (build tool, not a runtime dependency) | GPL-2.0-or-later with bootloader exception | [pyinstaller/pyinstaller](https://github.com/pyinstaller/pyinstaller) — [`licenses/pyinstaller-NOTICE.txt`](licenses/pyinstaller-NOTICE.txt) |

**Fan control reference** (`tools/fan-helper/smc_core.py`,
`tools/fan-test/fan_control.py`): the SMC struct layout and key
constants used to read/set fan speed were verified against the open,
MIT-licensed reference implementation
[MacFanControl](https://github.com/raminsharifi/MacFanControl) (Rust)
before being independently implemented in Python — not a direct port
or copy, but credited here since its reference code directly informed
ours. License: MIT — [`licenses/macfancontrol-MIT.txt`](licenses/macfancontrol-MIT.txt).

## Windows listener (`windows/`)

| Component | License | Source |
|---|---|---|
| github.com/getlantern/systray | Apache-2.0 | [getlantern/systray](https://github.com/getlantern/systray) — [`licenses/APACHE-2.0.txt`](licenses/APACHE-2.0.txt) |
| github.com/getlantern/context, errors, golog, hidden, ops | Apache-2.0 | getlantern org — [`licenses/APACHE-2.0.txt`](licenses/APACHE-2.0.txt) |
| github.com/getlantern/hex | BSD-style (Go Authors / Brave New Software) | [getlantern/hex](https://github.com/getlantern/hex) — [`licenses/getlantern-hex-BSD.txt`](licenses/getlantern-hex-BSD.txt) |
| github.com/go-stack/stack | MIT | [go-stack/stack](https://github.com/go-stack/stack) — [`licenses/go-stack-MIT.txt`](licenses/go-stack-MIT.txt) |
| github.com/oxtoacart/bpool | Apache-2.0 | [oxtoacart/bpool](https://github.com/oxtoacart/bpool) — [`licenses/APACHE-2.0.txt`](licenses/APACHE-2.0.txt) |
| golang.org/x/sys | BSD-3-Clause | [golang.org/x/sys](https://pkg.go.dev/golang.org/x/sys) — [`licenses/golang-x-sys-BSD.txt`](licenses/golang-x-sys-BSD.txt) |
| Go standard library (compiled into the listener) | BSD-3-Clause (The Go Authors) | [golang/go](https://github.com/golang/go) — same text as [`licenses/golang-x-sys-BSD.txt`](licenses/golang-x-sys-BSD.txt) |
| NSIS installer stub (inside `EiswolfsIBoardListener-Setup.exe`) | zlib/libpng | [nsis.sourceforge.io](https://nsis.sourceforge.io/License) — [`licenses/nsis-zlib-NOTICE.txt`](licenses/nsis-zlib-NOTICE.txt) |

All of the above are indirect dependencies pulled in by
`github.com/getlantern/systray` (the tray icon/menu library) — see
`windows/listener/go.mod`/`go.sum` for exact versions.

## Web installer (`index.html`, GitHub Pages)

| Component | License | Source |
|---|---|---|
| esp-web-tools (`esp-web-install-button`, loaded from unpkg.com) | Apache-2.0 (per public package index; verify against the upstream LICENSE file for the exact current terms) | [esphome/esp-web-tools](https://github.com/esphome/esp-web-tools) — [`licenses/APACHE-2.0.txt`](licenses/APACHE-2.0.txt) |

No other external scripts, fonts, or stylesheets are loaded by the web
installer page — everything else is inline.

## What the LGPL parts mean for the MIT license

Eiswolfs iBoard's own code is MIT. Two firmware components are LGPL: the
ESP32 Arduino core (LGPL-2.1) and AsyncTCP (LGPL-3.0, used by fauxmoESP).
Both are linked statically into `firmware.bin`. The LGPL does not change
the license of this project's own source, but anyone who receives the
compiled firmware must be able to rebuild it with a modified copy of those
libraries. That is satisfied here because the complete source is public
and the build uses the unmodified upstream libraries (`cd cyd && pio run`,
see `docs/DEVELOPMENT.md`). The LGPL license texts are included in
[`licenses/`](licenses/). The firmware binary as a whole is therefore
distributed under the combination of MIT (our code) and LGPL (those two
libraries); keep the license files with any redistribution.

## A note on this list

This list was compiled by reviewing `platformio.ini` (and the libraries
PlatformIO actually resolves into `cyd/.pio/libdeps`), `mac/requirements.txt`
and `mac/eiswolfs_iboard.spec`'s `hiddenimports`, `go.mod`/`go.sum`, and
the web installer's own `<script>` tags, plus the license metadata each
tool reports for itself. If you spot a missing or incorrect entry,
please open an issue.
