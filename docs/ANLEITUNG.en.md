# Eiswolfs iBoard – User Guide (English)

> **This is the single source of the guide.** All PDFs and copies are generated from this file (and its
> German sister file `ANLEITUNG.de.md`). Please make changes here only.
> `[Photo: …]` marks a placeholder for a picture, `[Generation]` and `[TODO: …]` will be filled in later.

Eiswolfs iBoard shows the live values of your Mac (CPU, GPU, temperatures, fan, power) on a small display,
the **CYD** (ESP32-2432S028, "Cheap Yellow Display"). In addition, the CYD can **wake up** (Wake-on-LAN)
and **shut down** a Windows laptop, switch your monitor's input, and listen to **Alexa**.

You do not have to set up everything. Start with chapters 1 to 4 and you already have the display with
your Mac's values. Everything else (waking the laptop, Alexa …) is optional.

**Contents**

1. [What you need](#1-what-you-need)
2. [Flash the CYD](#2-flash-the-cyd)
3. [WiFi and setup on the CYD](#3-wifi-and-setup-on-the-cyd)
4. [Install the Mac app](#4-install-the-mac-app)
5. [Create and enter the key](#5-create-and-enter-the-key)
6. [Fixed IP in the router](#6-fixed-ip-in-the-router-example-fritzbox)
7. [Wake-on-LAN on the laptop](#7-wake-on-lan-on-the-laptop)
8. [Windows listener](#8-windows-listener)
9. [Monitor switching](#9-monitor-switching)
10. [Alexa](#10-alexa)
11. [Troubleshooting](#11-troubleshooting)
12. [Security](#12-security)

---

## 1. What you need

**For the display (required):**

- A **CYD**, specifically the classic **ESP32-2432S028** with a touchscreen that reacts to pressure
  (resistive). A variant with a capacitive touchscreen is **not** suitable.
- A **microSD card** formatted as **FAT32**. The CYD stores its settings on it.
- A **USB cable that carries data** (not just power). If your computer does not recognize the CYD, the
  cable is the most common culprit.
- A computer with **Chrome, Edge or Opera** (for flashing in the browser).
- A **Mac with Apple Silicon** (M1 or newer) for the Mac app.
- A normal **2.4 GHz home WiFi**. A guest network usually does not work because guest networks separate
  devices from each other. The CYD cannot use 5 GHz networks.

**Only for the extras (optional):**

- A **Windows laptop**, if the CYD should switch it on and off.
- A **monitor with several inputs** (HDMI/DisplayPort), if it should switch when you wake the laptop.
- An **Alexa device** (for example an Echo Dot) and the Alexa app.
- Access to your **router settings** (example in chapter 6: FritzBox).

[Photo: All parts on a table – CYD, microSD card, USB cable]

---

## 2. Flash the CYD

"Flashing" means copying the firmware (the display's program) onto the CYD. You only do this once, and
again for new versions. The settings on the SD card are kept.

### Way A: web installer (recommended)

1. Put the **microSD card** into the CYD.
2. Connect the CYD to your computer via **USB**.
3. In **Chrome, Edge or Opera**, open
   **https://eiswolf-bg.github.io/Eiswolfs-iBoard-CYD/**.
4. Click **"Connect"** and pick the CYD from the list.
5. Confirm the installation. The rest runs by itself and takes one or two minutes. Do not unplug the cable.

[Photo: Web installer page with the "Connect" button]

If nothing shows up in the list, the **USB chip driver (CH340)** is usually missing:
[Windows](https://www.wch-ic.com/downloads/CH341SER_EXE.html) ·
[macOS](https://www.wch-ic.com/downloads/CH341SER_MAC_ZIP.html). On Linux it is normally built in.
Then replug the cable and try again.

### Way B: esptool (command line)

For anyone who cannot use the web installer. You need the file
**`Eiswolfs-iBoard-CYD-merged.bin`** from the project's releases page (it contains bootloader, partition
table and program in one file) and Python.

```
pip install esptool
esptool.py --chip esp32 --port <your-port> write_flash 0x0 Eiswolfs-iBoard-CYD-merged.bin
```

You find the port on a Mac with `ls /dev/cu.*`, on Windows in Device Manager (usually `COM3` or similar).

---

## 3. WiFi and setup on the CYD

On its very first start the CYD sets itself up on the display:

1. **Calibrate the touch:** Tap the four crosses one after the other (top left, top right, bottom right,
   bottom left). Then tap the **test point** in the middle.
2. **Choose WiFi:** Pick your **home WiFi** (2.4 GHz, no guest network) from the list. If it is not
   visible, tap "Search again" or "Enter manually".
3. **Enter the password:** With the on-screen keyboard. The password is shown in plain text right away, "Hide" switches to stars. The first letter is a capital, after that the keyboard continues in lower case. The key at the bottom left switches capitals on by hand.
4. When "Connected!" appears, the CYD is on your WiFi.
5. Finally it asks for the **language** (German or English).

[Photo: Calibration with the four crosses]
[Photo: WiFi list on the CYD]

The CYD and the Mac must be on the **same WiFi**.

**Changing something later:** **Swipe left on the main screen** to the Controls screen (page 2) and tap the **"Menu"** tile. (On the
"No connection to Mac" screen the button is also called **"Menu"**.) Tap **"Setup"**. It contains:

- **Change WiFi only** – choose another WiFi without redoing everything.
- **Recalibrate touch**
- **Brightness**
- **WoL devices** – set up laptops the CYD should wake and shut down (chapters 7 and 8).
- **Resync counter** – only needed if commands stop arriving after a reset (chapter 5).
- **Alexa** – voice control (chapter 10).

Next to every menu item there is a small **"?"**. Tap it for a short explanation.

[Photo: "Setup" menu on the CYD]

---

## 4. Install the Mac app

The Mac app lives in the menu bar and sends the values to the CYD.

1. Download **`Eiswolfs-iBoard.dmg`** from the releases page and open it with a double-click.
2. Drag the app **"Eiswolfs iBoard"** onto the **"Applications"** icon (in the German DMG: "Programme").
3. **First start (Gatekeeper):** The app is not signed with a paid Apple developer account, so macOS shows
   a warning. That is normal.
   - **Do not double-click.** Open the Applications folder, **right-click** (or two-finger click)
     "Eiswolfs iBoard" and choose **"Open"**. Confirm with **"Open"** once more in the dialog.
   - If only a message without an "Open" button appears: **System Settings → Privacy & Security**, scroll
     down to "Eiswolfs iBoard" and click **"Open Anyway"**.
   - You only need this on the first start.
4. **Allow the network:** On first start macOS asks whether the app may send on the **local network**.
   Answer **"Allow"**. Without it no values reach the CYD. To change it later: System Settings →
   Privacy & Security → Local Network.
5. **Start automatically:** Click the menu bar icon and tick **"Start at login"**.

[Photo: DMG window with app and "Applications"]
[Photo: Right-click → "Open"]
[Photo: Menu bar icon with the menu open]

After a few seconds the CYD shows your Mac's values. If it shows "No connection to Mac", see chapter 11.

**Note:** Not every Mac has all sensors. On Macs without a fan (for example a MacBook Air) the fan tile
permanently shows "--". That is not an error.

---

## 5. Create and enter the key

For **switching on and off** and a few extras (fan boost, sleep, shutting down Windows) the system uses a
**shared key**. It is a secret text you make up yourself. It must be **identical, character for
character,** in every place. It protects you from a stranger's device on your WiFi sending commands to
your computer.

The key never leaves your home network. Waking up via Wake-on-LAN and showing the values do **not** need
a key.

**Important:** Never put your key into a photo, a chat, a forum or a file you share. In this guide there
is only ever the placeholder `<YOUR-KEY>`.

### Create the key

On the Mac open **Terminal** and type:

```
openssl rand -hex 24
```

The output (48 characters made of digits and the letters a–f) is your key. Copy it. Alternatively any
random text of at least 30 characters will do.

### Enter the key

**a) Mac app.** Start the app once and quit it again (it then creates its settings file). Open the file
`~/Library/Application Support/Eiswolfs iBoard/config.json` with a text editor and enter:

```json
{
  "fan_boost_token": "<YOUR-KEY>"
}
```

The other entries in the file stay. Then start the app again.

**b) Windows listener** (only if the CYD should shut the laptop down): the easiest way is the Mac menu
**"Send token to Windows…"** (in the German app: "Token an Windows senden…"). It shows an address that is valid for **60 seconds**. Open it in the browser of the Windows
laptop. It downloads a `config.json` that contains **only the key**. **Do not overwrite the listener's own
file with it**, because that file also holds the device name, port and monitor settings, which would be lost.
Do this instead: right-click the listener icon → **"Einstellungen…"** (Settings) → next to the key click
**"From file…"** and pick the downloaded file. Only the key is taken over. Afterwards click **"Token neu
laden"** in the icon menu. Or type the key directly into the setup window (chapter 8).

**c) CYD firmware.** The key is part of the CYD's program (file `cyd/src/secrets.h`) and is built in when
the firmware is compiled:

```cpp
#pragma once
static const char* const FAN_BOOST_TOKEN = "<YOUR-KEY>";
```

Then build and flash the firmware yourself (`cd cyd && pio run --target upload`, see
`docs/DEVELOPMENT.md`).

> **[TODO: key in the prebuilt firmware.]** The prebuilt firmware from the web installer deliberately
> contains **no** real key. With that file fan boost, sleep and shutting down the laptop cannot be used.
> Waking up, the display, the menu and switching on via Alexa still work. Whether and how the key can be
> entered on the device itself is still to be decided and will be added here.

### If commands suddenly stop arriving ("resync")

Every command carries a counter that may only go up. If the CYD loses its counter (for example after a
reset), its commands are rejected. In that case open the counter resync on the receiver for **60 seconds**
(Mac menu → Security → "Allow counter resync (60s)…", Windows tray icon → "Zähler-Resync zulassen (60s)")
and right afterwards tap **"Resync counter"** in the CYD's Setup menu.

---

## 6. Fixed IP in the router (example FritzBox)

Waking up does not need an IP address. **Shutting down**, however, is sent by the CYD to one specific IP
address. So the laptop must **always have the same IP**, otherwise shutting down quietly stops working at
some point.

**Best done in the router (FritzBox):**

1. Open your FritzBox page (usually `http://fritz.box`) and log in.
2. **Home Network → Network**, and click the **pencil icon (edit)** next to the laptop.
3. Tick **"Always assign this network device the same IPv4 address"** and save.

[Photo: FritzBox, Home Network → Network, edit device]

Other routers call this "DHCP reservation" or "static lease".

**If your router cannot do that**, set a fixed IP on the device itself:

- **Windows:** Settings → Network & Internet → your connection → next to "IP assignment" click **Edit** →
  **Manual**. Enter an IP **outside** your router's DHCP range, plus subnet mask, gateway and DNS (usually
  both are the router's IP).
- **macOS:** System Settings → Network → your connection → Details → TCP/IP → "Configure IPv4: Manually".

### Find the IP and MAC address

You need both for the setup on the CYD.

- **Windows:** Open **Command Prompt** and type `ipconfig /all`. Find the active adapter (for a cable
  usually "Ethernet adapter Ethernet"). **"Physical Address"** is the MAC (six pairs like
  `AA-BB-CC-DD-EE-FF`), **"IPv4 Address"** is the IP.
- **macOS:** System Settings → Network → your connection → Details. The **Hardware** tab shows the MAC, the
  **TCP/IP** tab the IP.

[Photo: Output of ipconfig /all with MAC and IP marked]

**Tip for the Mac:** Turn off **"Private Wi-Fi Address"** for your network (System Settings → Wi-Fi → your
network → Details). A random address changes regularly and would also undo a fixed IP in the router.

---

## 7. Wake-on-LAN on the laptop

Wake-on-LAN ("WoL") wakes the laptop with a network signal. It must be switched on in **two** places.

1. **BIOS/UEFI:** Restart the laptop and enter the BIOS (depending on the maker the key F2, F10, Del or Esc
   right after power-on). Under the power settings look for **"Wake on LAN"** or **"Power On by
   PCI-E/PCIe"** and switch it **on**.
2. **Network adapter in Windows:** Device Manager → **Network adapters** → your adapter → **Properties**.
   - **Power Management** tab: tick **"Allow this device to wake the computer"**.
   - **Advanced** tab: set **"Wake on Magic Packet"** to **Enabled**, if listed.
3. **Turn off Fast Startup:** Control Panel → Power Options → "Choose what the power buttons do" → click
   "Change settings that are currently unavailable" → **untick "Turn on fast startup"**. Fast Startup puts
   the laptop into a state from which it often cannot be woken up.

[Photo: BIOS with "Wake on LAN"]
[Photo: Device Manager, "Power Management" tab]

**Connect the laptop with a network cable** if you can. Over WiFi a laptop usually cannot be woken from a
fully switched-off state, at most from sleep.

### Add the laptop on the CYD

1. On the CYD: **Menu (page 2, "Menu" tile) → Setup → "WoL devices" → "Add device"**.
2. Enter a **name** (your choice, shown on the tile).
3. Enter the **MAC address**, format `AA:BB:CC:DD:EE:FF` (from chapter 6).
4. Enter the **shutdown IP** (recommended, see chapter 6). If you skip it with "Back", the CYD can only
   wake the laptop, not shut it down.
5. Optional: choose **Monitor on power-on** (chapter 9).

Up to **4 devices** are possible. The first device sits at the top left of the **Controls** screen (swipe
left). From the second device on there is a second page, "Devices".

**Using it:** A **short tap** on the tile wakes the device. **Holding it** shuts it down (only with an
entered IP, the Windows listener installed and the key).

---

## 8. Windows listener

The listener is a small program on the Windows laptop. It receives the shut-down command from the CYD.
**You do not need it for waking up.**

1. Download **`EiswolfsIBoardListener-Setup.exe`** from the releases page and run it.
2. **SmartScreen:** Windows shows "Windows protected your PC" because the program is not signed with a paid
   certificate. That is normal. Click **"More info"** and then **"Run anyway"**. If you want to be sure,
   compare the SHA-256 checksum with the one on the releases page:
   `Get-FileHash EiswolfsIBoardListener-Setup.exe -Algorithm SHA256` (PowerShell).
3. **Administrator question (UAC):** Confirm once with **"Yes"**. The program uses it to create the
   **firewall rule** that lets the command through.
4. The setup window asks for:
   - **Device name:** your choice, just for orientation.
   - **Port:** leave it at **4212**. The CYD uses this port.
   - **Key:** the same as everywhere (chapter 5). Type it in, or use "From file…" to pick the file the
     Mac generated.
   - **Start automatically at login (autostart):** leave it ticked.
5. Second page **"Monitor switching":** see chapter 9. Skip it if you do not use monitor switching.

Afterwards an icon appears at the bottom right next to the clock (maybe behind the "Show hidden icons"
arrow). It shows the status ("Token gesetzt, bereit auf Port 4212") and has a menu with "Einstellungen…",
"Token neu laden", "Protokoll öffnen" and "Beenden". The menu is German-only.

**Firewall:** If the automatic rule did not work, a Windows window "Defender Firewall has blocked some
features" appears on the first command. Allow **only "Private networks"** there.

**Important:** The laptop's network must be classified as **"Private"**. After a restart Windows sometimes
switches it back to "Public", and then the firewall quietly blocks the command. Check with PowerShell:
`Get-NetConnectionProfile`.

**Updating:** Simply run the new setup file again. All your previous settings are pre-filled.
**Uninstalling:** Start menu → "Eiswolfs iBoard" → "Deinstallieren".

[Photo: SmartScreen warning with "More info"]
[Photo: Listener setup window]
[Photo: Tray icon with menu]

---

## 9. Monitor switching

If the Mac and the laptop share one monitor, the system can switch the monitor to the right **input**
(HDMI 1, HDMI 2, DisplayPort 1 or 2) when the laptop is woken up, and back when it is shut down. This is
optional.

**Requirement:** **DDC/CI** must be switched on in the **monitor** (it is in the monitor's on-screen menu,
often under "System" or "Other"). Some monitors, adapters or docks do not allow switching. You can only
find out by trying.

**How it works:**

- **Waking up:** The CYD wakes the laptop and tells the **Mac app** at the same time. The Mac app sets the
  chosen input through the Mac's monitor connection.
- **Shutting down:** The **Windows listener** switches the monitor back **before** it shuts the laptop
  down.

**Set it up:**

1. **Mac app:** Menu **"Monitor switching…"** → **"On"**. Choose the monitor under **"Screen"** (if nothing
   is listed: "Rescan screens"). With **"Test: switch to HDMI 2"** you can check that switching works. The
   monitor jumps briefly and returns by itself after 15 seconds. On some monitors the numbers for the
   inputs differ. You can adjust them under "Input values (VCP 0x60)".
2. **CYD:** In **WoL devices** (edit the laptop), in the last step **"Monitor on power-on"** choose: Off,
   HDMI 1, HDMI 2, DisplayPort 1 or DisplayPort 2.
3. **Windows listener:** On the **"Monitor switching"** page of the setup window switch it on, choose the
   monitor and the input value to switch **back** to.

[Photo: Mac menu "Monitor switching…"]
[Photo: Monitor on-screen menu with DDC/CI]

---

## 10. Alexa

With Alexa you can say: **"Alexa, turn on Laptop."** The CYD presents itself in your home network as a
light (Philips Hue, that is how Alexa sees it). Alexa finds it and triggers Wake-on-LAN. In the Alexa app
"Laptop" therefore shows up as a **light**, and "on" and "off" work like for a light.

> **Tested with:** Echo Dot [Generation]

**Important:** Alexa is **off** as delivered. As long as it is off, no additional service runs on the CYD
and no port is open.

### Turn it on

1. On the CYD: **Menu (page 2, "Menu" tile) → Setup → "Alexa"**.
2. Tap **"Enable Alexa"**. The button is filled when Alexa is on.
3. **Name:** Tap **"Name: …"**. The default is **Laptop**. Allowed are letters (without umlauts), digits,
   spaces and hyphens. Other characters are removed.
4. Tap **"Back"**. If you changed something, the **CYD restarts**. That is intended.
5. The name belongs to the **first device** in "WoL devices". It must be set up.

The notes in the menu (the small "?" next to each item) explain everything briefly.

[Photo: Alexa menu on the CYD]

### Search for devices

- Say **"Alexa, discover devices."** (in German: "Alexa, suche neue Geräte"). It takes about 20 seconds.
- The way through the app (**Devices → + → Add Device**) leads to a **list of manufacturers**. You will not find
  the iBoard there, use the voice command. If it does not work the first time, say it again and wait up to a
  minute, the Echo Dot's search sometimes needs two tries.
- Alexa tells you it found the device **"Laptop"** (or your name).

**After renaming** you have to **search again**. Delete the old device in the Alexa app first, otherwise it
stays twice.

### Commands

- **"Alexa, turn on Laptop."** wakes the laptop (Wake-on-LAN, with monitor switching if set up).
- **"Alexa, turn off Laptop."** shuts the laptop down **only if** you switched on **"Alexa: Shutdown"** in
  the Alexa menu (default: off). You need an entered IP, the Windows listener and the key for that
  (chapters 5 to 8).

In the Alexa app the switch jumps back to "off" by itself after a few seconds. That is intended: this way
"on" works every time, even if the laptop was on before.

### If it does not work

See the table in chapter 11. The most common reasons: the Alexa device and the CYD are not on the same
WiFi, the function is still off on the CYD, or the CYD was not restarted after the change.

---

## 11. Troubleshooting

| Symptom | Possible cause | Solution |
|---|---|---|
| The computer does not recognize the CYD | The cable carries power only, or the USB driver is missing | Use another (data) cable. Install the CH340 driver (chapter 2) |
| "No connection to Mac" | The Mac app is not running | Check that the menu bar icon is there and start the app |
| "No connection to Mac" | Mac and CYD are on different WiFis, or a guest network | Put both on the same normal home WiFi (2.4 GHz) |
| "No connection to Mac" | The macOS network permission is missing | System Settings → Privacy & Security → Local Network → allow "Eiswolfs iBoard" |
| The CYD does not connect to WiFi | Wrong password, or a 5 GHz network | The password is shown in plain text, check it carefully (upper and lower case). Use a 2.4 GHz network |
| The CYD shows "No SD card" | Card missing or not FAT32 | Insert a FAT32 card |
| Fan tile shows "--" | The Mac has no fan (for example MacBook Air) | Not an error |
| Waking up does not work | Wake-on-LAN is off in the BIOS or the network card | Recheck chapter 7, especially Fast Startup |
| Waking up does not work | The laptop is on WiFi | Connect it with a network cable |
| Waking up only works from sleep | Some laptops only wake from sleep, not from shut down | Test with the laptop fully shut down, use a cable if possible |
| Shutting down does not work but waking does | No IP entered on the CYD (holding then shows "Shutdown not set up") | Enter the IP under "WoL devices" (chapters 6, 7) |
| Shutting down worked, now it does not | The laptop's IP changed | Assign a fixed IP in the router (chapter 6), correct it on the CYD |
| Shutdown, fan boost or sleep do nothing | The key is not identical everywhere | Compare it character by character (chapter 5) |
| The listener log shows no new line after holding | The command does not reach the laptop (firewall, network) | Check the firewall rule "Eiswolfs iBoard Listener", network profile "Private" (chapter 8) |
| The log shows "invalid signature" | The key does not match, or the counters are out of step | Check the key. If it is right, do the counter resync |
| All commands are ignored after a CYD reset | The CYD's counter was reset | Resync (chapter 5) |
| Alexa does not find the device | Alexa is still off on the CYD | Menu → Setup → Alexa → "Enable Alexa", go back, the CYD restarts |
| Alexa does not find the device | The Alexa device and the CYD are on different WiFis | Both on the same home WiFi, no guest network |
| Alexa does not find the device | The router blocks discovery inside the home network (multicast) | Check the router setting for devices in the WiFi being allowed to talk to each other. Search again |
| Alexa finds nothing new after renaming | The old entry gets in the way, or nobody searched again | Delete the old device in the Alexa app, "Alexa, discover devices" |
| "Alexa, turn off Laptop" does nothing | "Alexa: Shutdown" is off (default) | Switch it on in the Alexa menu, IP, listener and key are also needed |
| Alexa says "Device is not responding" | The CYD only answers briefly and the switch jumps back | Usually it worked anyway. Check whether the laptop wakes. Otherwise search again |
| The monitor does not switch | DDC/CI is off on the monitor, or the monitor/adapter does not allow it | Switch DDC/CI on in the monitor menu. Check with "Test: switch to HDMI 2" in the Mac app |
| The monitor switches to the wrong input | The monitor's input value differs | Adjust it in the Mac app under "Input values (VCP 0x60)" |

---

## 12. Security

- **This is a home-network toy, not a security system.** Everything stays inside your WiFi. Nothing goes
  to the internet, the data only comes from your own devices.
- **The key is not encryption.** Commands travel through your WiFi as readable text. The key protects
  against someone **forging** commands or **replaying** an old command. Anyone listening in your network
  can still see **that** a command is being sent.
- **Waking up (Wake-on-LAN) has no protection.** It is a standard network signal without a key. Any device
  in your home network can wake your laptop.
- **Alexa:**
  - The device the CYD pretends to be has **no protection**. It uses the simple Philips Hue method without login.
    If Alexa is switched on on the CYD, **any device in your home network** can send "on" and "off".
  - If **"Alexa: Shutdown"** is on as well, **any device in your home network** can **shut the laptop
    down** without knowing the key. Only switch it on if you trust your home network.
  - Turn Alexa off when you do not need it. Then no service runs and no port is open.
- **Your home WiFi needs a good password.** Guests belong on the guest network.
- **Do not share your key** and do not show it in photos. If you published it by accident, create a new
  one (chapter 5) and enter it everywhere.
- **Fan boost** needs a small helper program with administrator rights (see
  `tools/fan-helper/README.md`). It is optional and only for advanced users.
- **The Windows listener** shuts the laptop down when a valid, signed command arrives. It stores the key
  only in your user folder (`%APPDATA%\EiswolfsIBoard\config.json`) and never writes it to the log.
- Please report security problems privately, see `SECURITY.md`.

More: `docs/TOKEN.md` explains the key and the replay protection in more detail.
