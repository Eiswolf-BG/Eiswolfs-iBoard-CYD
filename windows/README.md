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
No admin rights needed, no Windows service.

Its tray menu is German-only for now (it's a small, single-purpose
tool built for this project's primary user) — see
[`windows/listener/main.go`](listener/main.go) if you'd like to build
an English version, the texts are easy to translate.

## Installing

1. Run `EiswolfsIBoardListener-Setup.exe`. It installs to
   `%LOCALAPPDATA%\EiswolfsIBoardWin\`, adds a Start Menu group
   ("Eiswolfs iBoard") with a start and an uninstall shortcut, and adds
   a shortcut to your Startup folder (so it starts automatically every
   time you log in).
2. Set the [shared token](../docs/TOKEN.md) — easiest via the Mac app's
   "Token an Windows senden…" menu item (downloads a ready-made config
   file, no typing), or by hand in
   `%APPDATA%\EiswolfsIBoard\config.json`.
3. Click the tray icon → **"Token neu laden"** ("reload token") — picks
   up the new token immediately, no restart needed.

The tray menu also has:
- **Status line** — shows whether a token is set
- **Konfiguration öffnen** — opens the config folder in Explorer
- **Protokoll öffnen** — opens the log file in Notepad
- **Beenden** — quits the listener

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
GOOS=windows GOARCH=amd64 go build -ldflags "-H=windowsgui -s -w" \
    -o ../installer/EiswolfsIBoardListener.exe .

cd ../installer
makensis installer.nsi
```

Produces `EiswolfsIBoardListener-Setup.exe` in `windows/installer/`.
The tray icon (`icon.ico`, a simple ice-blue snowflake) is embedded
into the binary at build time via `go:embed` — there's a copy in both
`windows/listener/` (embedded into the exe) and `windows/installer/`
(bundled into the installer and used for the shortcuts).

## What it does, exactly

- Listens on UDP port 4212 (see `Config::WIN_SHUTDOWN_UDP_PORT` in
  `cyd/src/config.h`) for a line `SHUTDOWN <token>`.
- If the token matches `%APPDATA%\EiswolfsIBoard\config.json`'s
  `token` field, runs `shutdown /s /t 0 /f`.
- Any other line, or a wrong/missing token, is silently ignored.
- Logs to `%APPDATA%\EiswolfsIBoard\listener.log` (status messages
  only — never the token itself).
- Built with [`getlantern/systray`](https://github.com/getlantern/systray)
  for the tray icon/menu (pure Go on Windows, no CGO, cross-compiles
  cleanly from macOS/Linux).
