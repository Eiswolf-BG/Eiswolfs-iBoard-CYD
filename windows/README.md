# Eiswolfs iBoard — Windows shutdown listener

A tiny, optional background program for a Windows laptop that lets the
CYD shut it down remotely (long-press the CPU-temp tile). Waking it
back up uses plain Wake-on-LAN (short tap, same tile) — no software
needed on the Windows side for that part, it's a standard magic packet
your network card listens for even when the laptop is off.

This listener only handles the **shutdown** direction: it waits for a
small UDP command from the CYD and, if it carries the right
[shared token](../docs/TOKEN.md), runs Windows' own shutdown. It shows
a small icon in the system tray (bottom-right, next to the clock —
click the "show hidden icons" arrow if you don't see it right away)
and a Start Menu entry under "Eiswolfs iBoard", so it's easy to find.
It's fully modular — **no personal data of any kind ships in the
installer or the repository**: no token, device name, IP, port or
account name is baked in anywhere. Everything is entered by you, once,
in the setup wizard (or later, see "Changing settings" below), and
lives only in your own `%APPDATA%\EiswolfsIBoard\config.json`.

Its tray menu is German-only for now (it's a small, single-purpose
tool built for this project's primary user) — see
[`windows/listener/main.go`](listener/main.go) if you'd like to build
an English version, the texts are easy to translate.

## Installing

For a full walkthrough of setting up a *new* laptop from scratch
(static IP, finding its MAC address, enabling Wake-on-LAN, etc.), see
[docs/WOL_SETUP.md](../docs/WOL_SETUP.md). Short version, once the
device itself is ready:

1. Run `EiswolfsIBoardListener-Setup.exe`. It asks for **admin
   confirmation once** (a UAC prompt) — needed to set up the Windows
   Firewall rule automatically, see below.
2. The setup wizard asks for:
   - a **device name** (anything you like, shown in the tray status
     line and log, no functional meaning),
   - a **port** (defaults to 4212, matching the CYD — only change this
     if 4212 is already used by something else on this machine; the
     CYD doesn't currently support a different port per device, so
     changing it will break shutdown from the CYD until the firmware
     is updated too),
   - the **[shared token](../docs/TOKEN.md)** — the exact same secret
     string used by your CYD and Mac app. Paste/type it, or click
     "From file…" and pick a config file the Mac app already generated
     for you (Mac app menu → "Token an Windows senden…" — no typing
     needed either way),
   - whether to **start automatically at login** (on by default).
3. It installs to `%LOCALAPPDATA%\EiswolfsIBoardWin\`, adds a Start
   Menu group ("Eiswolfs iBoard") with start, settings and uninstall
   shortcuts, a Startup-folder shortcut (if you kept autostart on), an
   inbound Windows Firewall rule for the chosen port, writes your
   settings to `%APPDATA%\EiswolfsIBoard\config.json`, and starts the
   listener right away.
4. **Updating an existing installation** (e.g. a new release): just
   run the new Setup.exe again. It detects your existing settings
   (device name, port, token, autostart) and pre-fills them — nothing
   to re-enter, your CYD keeps working immediately. You can of course
   still change any field before continuing.

### Changing settings later

Start Menu → "Eiswolfs iBoard" → "Eiswolfs iBoard Listener
Einstellungen", or right-click the tray icon → **"Einstellungen..."**
("Settings...") — both open the exact same dialog as the initial
setup, pre-filled with your current values. The config file
(`%APPDATA%\EiswolfsIBoard\config.json`) stays hand-editable too, if
you prefer that; use the tray menu's **"Token neu laden"** ("reload
token") afterwards to pick up a manual edit without restarting.

The tray menu also has:
- **Status line** — shows whether a token is set, which port it's
  listening on, the device name, and any port error (e.g. "port
  already in use" instead of silently doing nothing)
- **Zähler-Resync zulassen (60s)** ("allow counter resync") — only
  needed if commands stop working after the CYD lost its own command
  counter (e.g. a factory reset). See [TOKEN.md](../docs/TOKEN.md#resync).
- **Alte, unsichere Befehle zulassen** ("allow old, unsigned commands")
  — off by default, a transition switch, see
  [TOKEN.md](../docs/TOKEN.md#replay-protection-what-the-token-actually-protects).
- **Konfiguration öffnen** — opens the config folder in Explorer
- **Protokoll öffnen** — opens the log file in Notepad
- **Beenden** — quits the listener

## Before you run it: SmartScreen and antivirus

This installer isn't code-signed (that needs a paid certificate this
hobby project doesn't have), so Windows will likely show a
"Windows protected your PC" SmartScreen warning, and some antivirus
tools may flag it too — both are standard for any unsigned .exe, not a
sign of anything actually wrong.

- **SmartScreen**: click "More info", then "Run anyway".
- **To double-check it's the exact file from this project** (not
  tampered with in transit): compare its SHA-256 checksum against the
  one listed on the [release page](https://github.com/Eiswolf-BG/Eiswolfs-iBoard-CYD/releases)
  for the version you downloaded. On Windows (PowerShell):
  ```
  Get-FileHash EiswolfsIBoardListener-Setup.exe -Algorithm SHA256
  ```
  On macOS/Linux: `shasum -a 256 EiswolfsIBoardListener-Setup.exe`
- **If you'd rather not trust a prebuilt binary at all**: the full
  source is right here — see "Building from source" below and build
  your own copy.

## Uninstalling

Start Menu → "Eiswolfs iBoard" → "Deinstallieren", or run
`Uninstall.exe` in the install folder
(`%LOCALAPPDATA%\EiswolfsIBoardWin\`) directly.

## Building from source

Requires [Go](https://go.dev/) 1.21+ and, for the installer,
[NSIS](https://nsis.sourceforge.io/) (`makensis`). Both can cross-build
a Windows `.exe` from macOS or Linux — no Windows machine needed to
build it, only to run it.

```
cd windows/listener
GOOS=windows GOARCH=amd64 go build -trimpath -buildvcs=false \
    -ldflags "-H=windowsgui -s -w" \
    -o ../installer/EiswolfsIBoardListener.exe .

cd ../installer
makensis installer.nsi
```

`-trimpath` strips your local build-machine file paths from the
binary — always use it when building a copy you intend to share.

Produces `EiswolfsIBoardListener-Setup.exe` in `windows/installer/`.
The tray icon (`icon.ico`, a simple ice-blue snowflake) is embedded
into the binary at build time via `go:embed` — there's a copy in both
`windows/listener/` (embedded into the exe) and `windows/installer/`
(bundled into the installer and used for the shortcuts).

## What it does, exactly

- Listens on UDP port 4212 (see `Config::WIN_SHUTDOWN_UDP_PORT` in
  `cyd/src/config.h`) for a line `SHUTDOWN <counter> <hmac>` (replay-
  protected command format, see [TOKEN.md](../docs/TOKEN.md)).
- If the HMAC matches (computed with `%APPDATA%\EiswolfsIBoard\
  config.json`'s `token` field) and the counter is higher than the
  last accepted one, runs `shutdown /s /t 0 /f`.
- Any other line, a wrong HMAC, or an old/repeated counter is silently
  ignored (no error echoed back to the CYD).
- Actually listens on the port from your config (the `port` field),
  falling back to 4212 if it's missing or invalid. If the configured
  port can't be opened (e.g. already in use by something else), it
  doesn't crash or exit — it logs the error, shows it in the tray
  status, and keeps retrying every 10 seconds, so you can still reach
  the tray icon to fix the setting.
- The installer creates an inbound Windows Firewall rule for the
  configured port automatically (needs the one-time admin confirmation
  mentioned above). If that ever fails, it falls back to the same
  manual path as before: accept Windows' own first-run "Defender
  Firewall has blocked some features" prompt, **"Private networks"**
  only.
- Only one instance can run at a time (a named Windows mutex) — safe
  to reopen the settings and have it relaunch the listener without
  ending up with two tray icons.
- Logs to `%APPDATA%\EiswolfsIBoard\listener.log` (status messages
  only — never the token itself).
- Built with [`getlantern/systray`](https://github.com/getlantern/systray)
  for the tray icon/menu (pure Go on Windows, no CGO, cross-compiles
  cleanly from macOS/Linux). The installer's input dialogs use NSIS's
  built-in `nsDialogs`, no extra dependency.
