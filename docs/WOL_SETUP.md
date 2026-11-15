# Setting up a new Wake-on-LAN device

*English below / Deutsch weiter unten.*

## English

A step-by-step guide for adding a new Windows laptop (or any other
WoL-capable device) to your CYD, from scratch.

### 1. Give it a fixed IP address

Wake-on-LAN itself doesn't need an IP (it's a MAC-address broadcast),
but **remote shutdown does** (the CYD sends that command straight to
one IP address) - the device needs the *same* IP every time, or
shutdown will quietly stop working after its IP changes.

**Preferred: reserve it in your router.** Most routers let you pin a
device's IP to its MAC address ("DHCP reservation" / "static lease").
On a FritzBox: **Heimnetz → Netzwerk → [device] → bearbeiten (edit
icon)** → check **"Diesem Netzwerkgerät immer die gleiche IPv4-Adresse
zuweisen"** ("Always assign this device the same IPv4 address"). Other
routers have an equivalent setting, usually under "DHCP" or "Network".

**If your router doesn't support that**, set a static IP directly on
the device instead:
- **Windows**: Settings → Network & Internet → your connection →
  "Edit" IP assignment → Manual → enter an IP outside your router's
  DHCP range (so it never gets handed to someone else), the subnet
  mask, gateway and DNS (usually your router's own IP for both).
- **macOS**: System Settings → Network → (your connection) → Details →
  TCP/IP → "Configure IPv4: Manually" → same values as above.

### 2. Find its MAC address and current IP

- **Windows**: open Command Prompt, run `ipconfig /all`, find your
  active network adapter (usually "Ethernet adapter Ethernet" for a
  cable connection) and note its **"Physical Address"** (the MAC, six
  pairs of hex digits like `AA-BB-CC-DD-EE-FF`) and **"IPv4 Address"**.
- **macOS**: System Settings → Network → (your connection) → Details →
  Hardware tab shows the MAC address; the Network tab shows the IP.

### 3. Turn on Wake-on-LAN

Needs to be enabled in **two** places on the Windows laptop:
- **BIOS/UEFI**: look for "Wake on LAN" or "Power On by PCI-E/PCIe"
  under power settings, enable it.
- **Network adapter**: Device Manager → Network adapters → your
  adapter → Properties → Power Management tab → check "Allow this
  device to wake the computer", and under the Advanced tab enable
  "Wake on Magic Packet" if listed.

**Use a wired (Ethernet) connection if you can.** WoL over WiFi is
unreliable at best on most laptops/routers - the WiFi adapter usually
can't listen for the magic packet once the laptop is fully powered
off, only while asleep. A cable connection wakes reliably from a full
shutdown.

### 4. On the Mac: turn off "Private Wi-Fi Address" for this network

Not relevant to the Windows laptop, but worth checking for the **Mac
mini** itself if you're setting up sleep/shutdown for it too: System
Settings → Wi-Fi → (your network) → Details → turn off "Private Wi-Fi
Address". A private (randomized) address changes periodically, which
would silently break anything relying on a fixed IP - including a
static-lease reservation in your router.

### 5. Install and set up the listener, add the device on the CYD

1. On the Windows laptop: download and run
   `EiswolfsIBoardListener-Setup.exe` from the
   [releases page](https://github.com/Eiswolf-BG/Eiswolfs-iBoard-CYD/releases)
   (see [windows/README.md](../windows/README.md) for the SmartScreen
   warning - normal for an unsigned installer, safe to click through).
2. The setup wizard asks for a device name (anything you like, just
   for your own reference), a port (leave at the default, 4212, unless
   it's already used by something else on that machine), and the
   **shared token** - the exact same secret string used by your CYD
   and Mac app (see [TOKEN.md](TOKEN.md)). Paste/type it, or use "From
   file…" to pick a config file the Mac app already generated for you
   (Mac app menu → "Token an Windows senden…"). Leave "Start
   automatically at login" checked.
3. The setup requests one-time administrator confirmation (UAC prompt)
   to add the Windows Firewall rule automatically - this is new and
   expected, approve it.
4. On the CYD itself: Menu → Setup → **"WoL devices"** → "Add device" →
   enter a name and the laptop's MAC address from step 2 (format
   `AA:BB:CC:DD:EE:FF`) → **strongly recommended:** also enter its IP
   address here (step 1/2 above) so shutdown is sent directly to that
   address instead of as a network broadcast - a plain broadcast is
   simpler to set up (no IP needed) but some routers/switches don't
   reliably deliver it, while a targeted packet almost always gets
   through → save.

That's it - tapping the new tile/button wakes the laptop; holding it
shuts it down (directly to the IP if you entered one, otherwise as a
broadcast - whichever device has an IP entered always uses it).

### 6. Troubleshooting

**Command doesn't arrive at all / nothing happens:**
- Check the listener's tray icon status (should say "Token gesetzt,
  bereit auf Port …") and its log (`listener.log`, tray menu →
  "Protokoll öffnen") for errors. Every received packet is logged with
  its sender address (e.g. "Befehl mit ungültiger Signatur von
  192.168.1.42:51000 verworfen") even if it's rejected - if you don't
  see *any* new line after pressing the button, the packet isn't
  reaching the listener at all (network/firewall issue, not a token
  issue).
- Make sure the token is *character-for-character* identical in
  `cyd/src/secrets.h`, the Mac app, and this listener - see
  [TOKEN.md](TOKEN.md).
- Confirm the Windows Firewall rule exists (Windows Defender Firewall
  with Advanced Security → Inbound Rules → "Eiswolfs iBoard Listener")
  and that the laptop is on the same network as the CYD.
- Confirm the laptop's active network connection is classified as
  "Private" (PowerShell: `Get-NetConnectionProfile`) - Windows
  sometimes reclassifies a home network as "Public" after a restart,
  which silently blocks the automatically-created firewall rule
  (switch it back under Settings → Network & Internet → your
  connection → Network profile type).
- If only *broadcast* (no IP entered on the CYD) fails while a
  targeted/unicast command works: enter the device's IP address on the
  CYD (see step 4 above) - broadcast delivery depends on your specific
  router/switch and isn't always reliable, a targeted packet sidesteps
  that entirely.

**WoL (wake-up) doesn't work:**
- Check steps 1-3 above again, especially BIOS and adapter settings -
  this is almost always the cause, not a CYD/network problem.
- Try it with the laptop fully shut down (not just asleep) - some
  laptops only wake from sleep, not from a full power-off, even with
  WoL enabled.
- WiFi-only WoL is unreliable - use a cable if at all possible (see
  step 3).

**Shutdown doesn't work (but WoL does):**
- Confirm you entered the laptop's IP address under its entry in "WoL
  devices" on the CYD - without an IP, holding the button shows
  "Shutdown not set up" and sends nothing, by design.
- Check that the IP hasn't changed (see step 1) - a dynamic IP that
  moved is the most common cause.

---

## Deutsch

Schritt-für-Schritt-Anleitung, um einen neuen Windows-Laptop (oder ein
anderes WoL-fähiges Gerät) von Grund auf am CYD einzurichten.

### 1. Feste IP-Adresse vergeben

Wake-on-LAN selbst braucht keine IP (das ist ein MAC-Adress-Broadcast),
aber das **Ausschalten aus der Ferne schon** (der CYD schickt diesen
Befehl gezielt an eine IP-Adresse) - das Gerät braucht also *immer*
dieselbe IP, sonst hört das Ausschalten irgendwann einfach auf zu
funktionieren, sobald sich die IP ändert.

**Bevorzugt: im Router reservieren.** Die meisten Router erlauben es,
die IP eines Geräts an seine MAC-Adresse zu binden ("DHCP-Reservierung"
/ "feste IP-Vergabe"). Bei einer FritzBox: **Heimnetz → Netzwerk →
[Gerät] → bearbeiten (Stift-Symbol)** → **"Diesem Netzwerkgerät immer
die gleiche IPv4-Adresse zuweisen"** anhaken. Andere Router haben eine
vergleichbare Einstellung, meist unter "DHCP" oder "Netzwerk".

**Falls dein Router das nicht kann**, direkt am Gerät eine feste IP
einstellen:
- **Windows**: Einstellungen → Netzwerk und Internet → deine Verbindung
  → "Bearbeiten" bei IP-Zuweisung → Manuell → eine IP AUSSERHALB des
  DHCP-Bereichs deines Routers eintragen (sonst vergibt der Router sie
  evtl. zusätzlich an jemand anderen), dazu Subnetzmaske, Gateway und
  DNS (meist beides die IP deines Routers).
- **macOS**: Systemeinstellungen → Netzwerk → (deine Verbindung) →
  Details → TCP/IP → "IPv4 konfigurieren: Manuell" → gleiche Werte wie
  oben.

### 2. MAC-Adresse und aktuelle IP herausfinden

- **Windows**: Eingabeaufforderung öffnen, `ipconfig /all` eingeben,
  den aktiven Netzwerkadapter suchen (bei Kabel meist "Ethernet-Adapter
  Ethernet") und dort die **"Physische Adresse"** (die MAC, sechs
  Hex-Zahlenpaare wie `AA-BB-CC-DD-EE-FF`) und **"IPv4-Adresse"**
  notieren.
- **macOS**: Systemeinstellungen → Netzwerk → (deine Verbindung) →
  Details → Reiter "Hardware" zeigt die MAC-Adresse, Reiter "Netzwerk"
  die IP.

### 3. Wake-on-LAN einschalten

Muss an **zwei** Stellen am Windows-Laptop aktiviert werden:
- **BIOS/UEFI**: unter den Energieeinstellungen nach "Wake on LAN"
  oder "Power On by PCI-E/PCIe" suchen und aktivieren.
- **Netzwerkkarte**: Geräte-Manager → Netzwerkadapter → dein Adapter →
  Eigenschaften → Reiter "Energieverwaltung" → "Gerät kann den
  Computer aus dem Ruhezustand aktivieren" anhaken, im Reiter
  "Erweitert" zusätzlich "Wake on Magic Packet" aktivieren, falls
  vorhanden.

**Wenn möglich per Kabel (Ethernet) verbinden.** WoL übers WLAN ist bei
den meisten Laptops/Routern unzuverlässig - die WLAN-Karte kann das
Magic Packet meist nur im Ruhezustand empfangen, nicht mehr nach einem
vollständigen Ausschalten. Eine Kabelverbindung weckt zuverlässig auch
aus dem kompletten Ausschalten.

### 4. Am Mac: "Private WLAN-Adresse" für dieses Netz ausschalten

Betrifft nicht den Windows-Laptop, aber lohnt sich zu prüfen, falls du
auch für den **Mac mini selbst** Ruhezustand/Ausschalten einrichtest:
Systemeinstellungen → WLAN → (dein Netzwerk) → Details → "Private
WLAN-Adresse" ausschalten. Eine private (zufällige) Adresse ändert sich
regelmäßig, was alles stillschweigend kaputt macht, was sich auf eine
feste IP verlässt - auch eine Reservierung im Router.

### 5. Listener installieren und einrichten, Gerät am CYD anlegen

1. Am Windows-Laptop: `EiswolfsIBoardListener-Setup.exe` von der
   [Releases-Seite](https://github.com/Eiswolf-BG/Eiswolfs-iBoard-CYD/releases)
   herunterladen und ausführen (siehe [windows/README.md](../windows/README.md)
   zum SmartScreen-Hinweis - normal bei einem unsignierten Installer,
   kann bedenkenlos durchgeklickt werden).
2. Die Einrichtung fragt nach einem Gerätenamen (frei wählbar, nur zur
   eigenen Übersicht), einem Port (Vorgabe 4212 so lassen, außer er ist
   auf diesem Rechner schon belegt) und dem **gemeinsamen Schlüssel** -
   demselben Geheimtext, den auch CYD und Mac-App benutzen (siehe
   [TOKEN.md](TOKEN.md)). Eintippen/einfügen, oder über "Aus Datei
   wählen…" eine config-Datei auswählen, die die Mac-App bereits für
   dich erzeugt hat (Mac-App-Menü → "Token an Windows senden…"). "Beim
   Anmelden automatisch starten" angehakt lassen.
3. Die Einrichtung fragt einmalig nach Administrator-Bestätigung
   (UAC-Fenster), um die Windows-Firewall-Regel automatisch
   anzulegen - das ist neu und erwartet, einfach bestätigen.
4. Am CYD selbst: Menü → Einrichtung → **"WoL-Geräte"** → "Gerät
   hinzufügen" → Namen und die MAC-Adresse des Laptops aus Schritt 2
   eintragen (Format `AA:BB:CC:DD:EE:FF`) → **dringend empfohlen:**
   zusätzlich dessen IP-Adresse eintragen (Schritt 1/2 oben), damit das
   Ausschalten gezielt an diese Adresse geschickt wird statt als
   Netzwerk-Broadcast - ein reiner Broadcast ist zwar einfacher (keine
   IP noetig), aber manche Router/Switches liefern ihn nicht
   zuverlaessig aus, waehrend ein gezieltes Paket so gut wie immer
   ankommt → speichern.

Fertig - Antippen der neuen Kachel/des Buttons weckt den Laptop,
Halten schaltet ihn aus (direkt an die IP, falls eine eingetragen
wurde, sonst als Broadcast - sobald bei einem Geraet eine IP
eingetragen ist, wird sie immer benutzt).

### 6. Fehlersuche

**Befehl kommt gar nicht an / nichts passiert:**
- Status im Taskleisten-Symbol des Listeners prüfen (sollte "Token
  gesetzt, bereit auf Port …" zeigen) und das Protokoll
  (`listener.log`, Taskleisten-Menü → "Protokoll öffnen") auf
  Fehlermeldungen durchsehen. Jedes empfangene Paket wird mit der
  Absenderadresse geloggt (z.B. "Befehl mit ungültiger Signatur von
  192.168.1.42:51000 verworfen"), auch wenn es abgelehnt wird - steht
  nach einem Tastendruck GAR KEINE neue Zeile im Protokoll, kommt das
  Paket gar nicht erst beim Listener an (Netzwerk-/Firewall-Problem,
  kein Token-Problem).
- Sicherstellen, dass der Schlüssel in `cyd/src/secrets.h`, der
  Mac-App und diesem Listener *zeichengenau* identisch ist - siehe
  [TOKEN.md](TOKEN.md).
- Prüfen, dass die Windows-Firewall-Regel existiert (Windows Defender
  Firewall mit erweiterter Sicherheit → Eingehende Regeln → "Eiswolfs
  iBoard Listener") und dass der Laptop im selben Netz wie das CYD ist.
- Prüfen, dass die aktive Netzwerkverbindung des Laptops als "Privat"
  eingestuft ist (PowerShell: `Get-NetConnectionProfile`) - Windows
  stuft ein Heimnetz nach einem Neustart manchmal wieder als
  "Öffentlich" ein, wodurch die automatisch angelegte Firewall-Regel
  still blockiert wird (zurückstellen unter Einstellungen → Netzwerk
  und Internet → deine Verbindung → Netzwerkprofiltyp).
- Falls nur der **Broadcast**-Weg (keine IP am CYD eingetragen)
  fehlschlägt, ein gezielter Befehl aber funktioniert: die IP-Adresse
  des Geräts am CYD eintragen (siehe Schritt 4 oben) - die
  Broadcast-Zustellung hängt vom jeweiligen Router/Switch ab und ist
  nicht immer zuverlässig, ein gezieltes Paket umgeht das komplett.

**Einschalten (WoL) funktioniert nicht:**
- Schritte 1-3 oben nochmal prüfen, besonders BIOS- und
  Adapter-Einstellungen - das ist fast immer die Ursache, kein
  CYD-/Netzwerkproblem.
- Mit komplett ausgeschaltetem (nicht nur schlafendem) Laptop testen -
  manche Laptops wecken trotz aktiviertem WoL nur aus dem Ruhezustand,
  nicht aus dem vollständigen Ausschalten.
- WLAN-only-WoL ist unzuverlässig - wenn möglich Kabel nutzen (siehe
  Schritt 3).

**Ausschalten funktioniert nicht (Einschalten aber schon):**
- Prüfen, ob die IP-Adresse des Laptops beim Eintrag in "WoL-Geräte"
  am CYD eingetragen wurde - ohne IP zeigt Halten absichtlich
  "Ausschalten nicht eingerichtet" und sendet nichts.
- Prüfen, ob sich die IP-Adresse geändert hat (siehe Schritt 1) - eine
  verrutschte dynamische IP ist die häufigste Ursache.
