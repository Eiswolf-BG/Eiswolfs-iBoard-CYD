# Security Policy

Eiswolfs iBoard is a hobby project, not a commercial product — see
the realistic expectations in [LICENSE](LICENSE) ("as is", no warranty). That said, reports about real security issues
are welcome and taken seriously.

## Supported versions

Only the **latest released version** (see the
[releases page](https://github.com/Eiswolf-BG/Eiswolfs-iBoard-CYD/releases))
is supported. Please update before reporting an issue, in case it's
already fixed.

## Reporting a vulnerability

Please use GitHub's private vulnerability reporting instead of a
public issue: go to the
[Security tab](https://github.com/Eiswolf-BG/Eiswolfs-iBoard-CYD/security)
of this repository and click **"Report a vulnerability"**. This keeps
the details private between you and the maintainer until a fix is
ready.

If that's not available for some reason, open a regular GitHub issue
with as few technical specifics as possible and ask to be contacted
privately — please don't post exploit details, your shared token, or
any other sensitive configuration in a public issue.

## If you suspect your shared token leaked

Fan boost, sleep, and the Windows shutdown feature are all protected
by a [shared secret token](docs/TOKEN.md) with replay protection — but
if you think it's been exposed (e.g. accidentally committed, shared,
or captured), the fix is simple: **generate a new one** and set it
again in all the places you use it (see
[docs/TOKEN.md](docs/TOKEN.md)). There's nothing to revoke or
coordinate online — it's purely local to your own devices.

## Scope

This project controls a few things on your own local network (fan
speed limits, sleep, shutdown) — see
[docs/TOKEN.md](docs/TOKEN.md#replay-protection-what-the-token-actually-protects)
for an honest description of what the token protects against and what
it doesn't. It is not designed to resist a determined attacker who
already has access to your home network.
