# Fan Helper (optional)

A tiny background helper that lets the Mac app push the fan to a fixed
speed ("fan boost" - from the Mac's menu bar, or by holding the FAN
tile on the CYD). **Entirely optional**: without it, Eiswolfs iBoard
still shows fan *speed* normally - you just can't *control* it, and
the fan-boost button/tile will say "not available".

## Why does this need root at all?

Reading the fan's current RPM needs no special privileges (the Mac app
does that on its own, via [macmon](https://github.com/vladkens/macmon)).
*Setting* a fan's target speed is different: on Apple Silicon Macs that
goes through the SMC (System Management Controller), and only root can
write to it. The Mac app itself intentionally never runs as root - so
this one small, separate piece does, and nothing else does.

`fand.py` is deliberately tiny and does *only* this one thing: it
listens on a local Unix socket (never reachable over the network, only
from this Mac, and only your own user account can even open it) for
four possible commands (`PING`, `STATUS`, `BOOST <rpm>`, `AUTO`), and
refuses anything else. Built-in safety limits, independent of whatever
asks for a boost:

- Never below the fan's own reported minimum (never actually "off")
- Never above its reported maximum
- A watchdog: boost active + no refresh from the Mac app for more than
  10 seconds → automatically back to automatic
- A hard 4-hour cap per boost, even in "until stopped" mode
- Reverts to automatic on its own exit, on a stop signal, and at every
  startup

## Where do I get it?

You already have it - it's this folder, part of the main repo you
cloned/downloaded (`tools/fan-helper/`). There's nothing separate to
download.

## Installing

```
cd tools/fan-helper
sudo bash install.sh
```

That's it - no prompts beyond the `sudo` password itself (which macOS
asks for, this script never stores or sees it). It installs `fand.py`
as a `LaunchDaemon` (so it starts automatically on every boot, even
before login) and starts it immediately. Fan boost in the Mac app's
menu and on the CYD will say "available" right after.

## Uninstalling

```
cd tools/fan-helper
sudo bash uninstall.sh
```

Stops the service, removes it from startup, and puts the fan back on
automatic on the way out.

## Checking it's running

```
sudo launchctl print system/com.alexdemurtas.eiswolfsiboard.fanhelper
```

Logs: `/Library/Logs/eiswolfsiboard-fanhelper.log` and `.err.log`.
