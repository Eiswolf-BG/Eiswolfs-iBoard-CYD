# The shared token, explained

Fan boost, sleep, and the Windows shutdown listener are gated by a
**shared secret token** — a string you make up yourself, that has to be
*exactly identical* in three places:

1. `cyd/src/secrets.h` (CYD firmware, compiled in)
2. The Mac app's config file
3. The Windows listener's config file (only if you use the optional
   Windows shutdown feature)

It's not a password for anything online — it never leaves your local
network. Its only job is to stop a random device on your WiFi from
accidentally (or deliberately) sending your Mac to sleep, boosting the
fan, or shutting down your Windows laptop.

Wake-on-LAN (waking the Windows laptop) does **not** use this token —
it's a plain WoL magic packet, which has no concept of authentication
at all (see [README.md](../README.md)).

## Replay protection (what the token actually protects)

**This is not encryption.** Every command the CYD sends (fan boost,
sleep, shutdown) still travels as plain, readable text over WiFi/USB —
anyone capturing your network traffic can see exactly what command was
sent. What the token *does* protect against: someone **forging** a
command (they'd need the token to produce a valid signature) or
**replaying** a captured command later to trigger it again (each
command carries a counter that only ever goes up; a repeated or
out-of-order counter is silently ignored).

Technically: each command ends in `<counter> <hmac>` instead of the
raw token — `hmac` is the first 16 hex characters of an HMAC-SHA256
over the command text and counter, keyed with your token. The CYD
keeps its own counter on its SD card/internal storage and always sends
a higher one than last time; the receiving side (Mac app, Windows
listener) remembers the highest counter it has accepted and rejects
anything equal or lower. The token itself never travels over the
network at all anymore — only this signature, which doesn't reveal it.

**Limits, honestly**: this protects against casual/opportunistic
replay on your own LAN, not against a determined attacker who's
already on your network and watching in real time (they can still see
*when* you send a command, just not forge a new one or make an old one
fire again). It's proportionate to the actual risk here — a home
automation convenience feature, not a safety-critical system.

### Resync

If the CYD ever loses its own counter (most likely after a factory
reset) and fan boost/sleep/shutdown stop responding, each receiver has
a matching "allow resync" switch, open for 60 seconds:

- **Mac app**: menu → Security → "Allow counter resync (60s)…"
- **Windows listener**: tray icon → "Zähler-Resync zulassen (60s)"

Open it, then immediately tap "Resync counter" in the CYD's own menu
(Setup page) — within that window, the receiver accepts the CYD's new
counter as the fresh baseline, no matter its value. Outside that
window, a RESYNC command is treated like any other and ignored if it
isn't higher than before. This has to be a deliberate, physical action
on the receiving side by design — the CYD alone can never prove its
new counter is legitimately higher after losing its history.

### Old firmware / app during the upgrade

If you updated the CYD firmware but haven't updated the Mac app and/or
Windows listener yet (or the other way around), commands will simply
stop working (old clear-text tokens are rejected by new receivers, and
vice versa) rather than silently falling back to something less safe.
Each receiver has an **"allow old, unsigned commands"** switch (off by
default) for a short transition if you really can't update everything
at once — turning it on removes replay protection entirely for that
receiver until you turn it back off, so only use it briefly.

## Picking a token

Any sufficiently random string works — it only has to be hard to guess
by accident, not resist a determined attacker on your own LAN. A quick
way to generate one on a Mac or Linux machine:

```
openssl rand -hex 24
```

Or just mash the keyboard for 30+ random characters. Whatever you pick,
you'll copy the *exact same string* into all the places below.

## 1. CYD firmware (`cyd/src/secrets.h`)

This file doesn't exist in the repo (it's gitignored on purpose — never
commit your token). Create it yourself:

```cpp
#pragma once
static const char* const FAN_BOOST_TOKEN = "your-random-string-here";
```

Then build and flash normally (`cd cyd && pio run --target upload`).

## 2. Mac app

The Mac app has no settings screen for this yet — edit its config file
directly with any text editor:

```
~/Library/Application Support/Eiswolfs iBoard/config.json
```

Add (or edit) the `fan_boost_token` field:

```json
{
  "fan_boost_token": "your-random-string-here"
}
```

(If the file doesn't exist yet, start the app once first — it creates
the file with defaults on first launch, then quit the app, edit the
file, and start it again.)

## 3. Windows listener (optional, for the shutdown feature)

Only needed if you're using the optional Windows laptop shutdown
feature (`windows/`, see [windows/README.md](../windows/README.md)).

**Easiest way** (Mac app menu bar → "Token an Windows senden…" /
"Send token to Windows…", depending on your chosen app language):
shows a local-network address, valid for 60 seconds - open it in a
browser on the Windows laptop, and it downloads a ready-made
`config.json` that contains **only the token**. **Do not overwrite
your listener's `config.json` with it** - that file also holds the
device name, port and monitor-switching settings, which would be lost.
Instead open the listener's tray menu → "Einstellungen..." and use
**"From file..." / "Aus Datei wählen…"** to pick the downloaded file:
only the token is taken over, everything else stays. (On a fresh
install, where there are no other settings yet, copying the file to
`%APPDATA%\EiswolfsIBoard\config.json` works too.) Then click "Token
neu laden" in the tray menu - no typing involved.

**Manual way**, if you'd rather not use that (or on a Mac without the
app running): edit the file yourself with any text editor:

```
%APPDATA%\EiswolfsIBoard\config.json
```

```json
{
  "token": "your-random-string-here"
}
```

Either way, open the listener's tray icon menu and click "Token neu
laden" afterward - no restart needed. (The listener's own menu is
German-only for now - it's a small, single-purpose tool built for this
project's primary user; the texts in `windows/listener/main.go` are
easy to translate if you want an English build.)

## If something doesn't work

A mismatched or missing token doesn't cause an error message anywhere
on purpose — the receiving side just silently ignores the command, the
same as if the feature were switched off. If fan boost, sleep, or
Windows shutdown don't do anything when you trigger them from the CYD,
double-check the token is *character-for-character* identical in all
the places you're using.
