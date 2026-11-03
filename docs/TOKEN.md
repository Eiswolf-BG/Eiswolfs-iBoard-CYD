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
accidentally (or deliberately) sending your Mac to sleep or shutting
down your Windows laptop by guessing a plain command. The CYD attaches
it to every command it sends; the receiving side only acts on the
command if the token matches.

Wake-on-LAN (waking the Windows laptop) does **not** use this token —
it's a plain WoL magic packet, which has no concept of authentication
at all (see [README.md](../README.md)).

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
`config.json` with the token already filled in. Move the downloaded
file to `%APPDATA%\EiswolfsIBoard\config.json` (replacing the empty
one the installer created) and you're done - no typing involved.

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
