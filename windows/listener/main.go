// Eiswolfs iBoard - Windows-Hintergrund-Listener (Auftrag "Windows-Laptop
// herunterfahren"). Gegenstueck zur Mac-App: lauscht per UDP auf
// Config::WIN_SHUTDOWN_UDP_PORT (siehe cyd/src/config.h, Port 4212) auf
// signierte Befehle vom CYD und loest bei gueltiger Signatur Windows'
// eigenen Shutdown aus.
//
// Taskleisten-Symbol + Menue (Auftrag "das muss man finden koennen") -
// zeigt den Token-Status, erlaubt ein Neuladen der config.json OHNE
// Prozess-Neustart (loest Alex' eigentliches Problem: Token ersetzt,
// aber Listener kannte ihn noch nicht) und einen "Beenden"-Knopf.
//
// Schutz gegen Befehlswiederholung (Auftrag "Sicherheit und Lizenzen",
// Teil 2) - erwartet jetzt "SHUTDOWN <Zaehler> <16-Hex-Zeichen-HMAC>"
// statt des bisherigen Klartext-Tokens am Zeilenende, siehe
// verifySigned() unten (identische Berechnung wie cyd/src/cmd_auth.cpp
// und mac/fan_boost.py::_verify_signed()). Der Token steht NIE im
// Quellcode - er wird beim ersten Start in
// %APPDATA%\EiswolfsIBoard\config.json mit einem leeren Platzhalter
// angelegt; Alex traegt denselben Token ein, den auch die Mac-App und
// das CYD benutzen (siehe docs/TOKEN.md). Ohne gesetzten Token
// ignoriert der Listener jeden Befehl.
package main

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	_ "embed"
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"
	"unsafe"

	"github.com/getlantern/systray"
)

// Schutz gegen doppelte Instanzen (Auftrag "Modularer Listener") - noetig,
// seit das Setup den Listener nach einer Einstellungsaenderung neu
// startet (vorher konnte es nie zwei gleichzeitig laufende Kopien
// geben). Benanntes Windows-Mutex statt einer Sperrdatei - braucht
// keine zusaetzliche Abhaengigkeit (reiner syscall-Aufruf), gleiches
// Prinzip wie mac/sender.py::acquire_single_instance_lock() im
// Schwesterprojekt.
func acquireSingleInstanceLock() bool {
	kernel32 := syscall.NewLazyDLL("kernel32.dll")
	createMutexW := kernel32.NewProc("CreateMutexW")
	namePtr, err := syscall.UTF16PtrFromString("EiswolfsIBoardListenerSingleInstance")
	if err != nil {
		return true // im Zweifel weiterlaufen, nicht blockieren
	}
	handle, _, callErr := createMutexW.Call(0, 0, uintptr(unsafe.Pointer(namePtr)))
	if handle == 0 {
		return true // Mutex konnte nicht erzeugt werden - im Zweifel weiterlaufen
	}
	const errorAlreadyExists syscall.Errno = 183
	return callErr != errorAlreadyExists
}

const (
	// Standard-Port (Auftrag "Modularer Listener") - MUSS zu
	// Config::WIN_SHUTDOWN_UDP_PORT in cyd/src/config.h passen, sofern
	// er nicht in der config.json ueberschrieben wird. WICHTIG: das CYD
	// kennt nur EINEN festen Port fuer ALLE WoL-Geraete (kein Feld pro
	// Geraet) - wird dieser Listener auf einen anderen Port umgestellt,
	// kann das CYD ihn nicht mehr erreichen, bis die Firmware ebenfalls
	// angepasst wird. Nur aendern, wenn 4212 auf diesem Rechner belegt
	// ist.
	defaultUdpPort = 4212
	hmacHexLen     = 16
	resyncWindowS  = 60
)

//go:embed icon.ico
var iconData []byte

// DeviceName/Port/Autostart neu (Auftrag "Modularer Listener") - vorher
// nur Token/Zaehler/Legacy-Schalter. Port 0/fehlend faellt auf
// defaultUdpPort zurueck (siehe effectivePort()), DeviceName ist rein
// informativ (Tray-Tooltip/Log), Autostart wird vom Installer selbst
// umgesetzt (Verknuepfung im Autostart-Ordner) - hier nur gespeichert,
// damit die Einstellungen-Seite ihn anzeigen/aendern kann.
type config struct {
	Token               string `json:"token"`
	LastCmdCounter      uint32 `json:"last_cmd_counter"`
	AllowLegacyCommands bool   `json:"allow_legacy_commands"`
	DeviceName          string `json:"device_name"`
	Port                int    `json:"port"`
	Autostart           bool   `json:"autostart"`
	// Auftrag "Monitor-Umschaltung" - additiv, Standard AUS (false/0/"").
	// MonitorVcpValue ist der rohe VCP-0x60-Wert (nicht der logische
	// CYD-Eingang 1-4 - der Windows-Listener kennt nur EINEN Zielwert,
	// keine Zuordnungstabelle, da er immer nur "zurueck auf den Mac-
	// Eingang" schaltet). MonitorScreenNameSubstring ist ein Teilstring
	// des von Windows gemeldeten Monitornamens (z.B. "AW2720HFA") - LEER
	// bedeutet "kein Monitor ausgewaehlt", es wird dann NICHTS getan
	// (gleiche Sicherheitsgrenze wie bei der Mac-App - kein Fallback auf
	// "irgendeinen"/"den ersten gefundenen" Monitor).
	MonitorSwitchEnabled       bool   `json:"monitor_switch_enabled"`
	MonitorVcpValue            int    `json:"monitor_vcp_value"`
	MonitorScreenNameSubstring string `json:"monitor_screen_name_substring"`
}

func effectivePort(c config) int {
	if c.Port <= 0 || c.Port > 65535 {
		return defaultUdpPort
	}
	return c.Port
}

var (
	cfgMu              sync.Mutex
	cfg                config
	mStatus            *systray.MenuItem
	mLegacy            *systray.MenuItem
	resyncUntilMonoSec float64
	resyncMu           sync.Mutex
	startMono          = time.Now()
	portErrMu          sync.Mutex
	portErr            string
)

func setPortError(msg string) {
	portErrMu.Lock()
	portErr = msg
	portErrMu.Unlock()
	updateStatus()
}

func monoSeconds() float64 {
	return time.Since(startMono).Seconds()
}

func configPath() string {
	appData := os.Getenv("APPDATA")
	if appData == "" {
		appData = "."
	}
	return filepath.Join(appData, "EiswolfsIBoard", "config.json")
}

// Legt die Config-Datei mit leerem Token an, falls sie noch nicht
// existiert - ueberschreibt eine vorhandene Datei NIE beim Anlegen
// (gleiches Prinzip wie mac/config.py::_load()).
func loadConfig() config {
	path := configPath()
	data, err := os.ReadFile(path)
	if err == nil {
		var c config
		if json.Unmarshal(data, &c) == nil {
			return c
		}
	}

	os.MkdirAll(filepath.Dir(path), 0700)
	// Autostart-Vorgabe "an" (Auftrag) - der Installer legt die
	// tatsaechliche Verknuepfung an, dieses Feld spiegelt das nur fuer
	// die Einstellungen-Seite.
	c := config{Autostart: true}
	if out, err := json.MarshalIndent(c, "", "  "); err == nil {
		os.WriteFile(path, out, 0600)
	}
	return c
}

// Schreibt den KOMPLETTEN, zusammengefuehrten Zustand zurueck (erhaelt
// dabei auch Felder wie "token", die hier nie geaendert werden) -
// gleiches Muster wie mac/config.py::_save().
func saveConfigLocked() {
	path := configPath()
	out, err := json.MarshalIndent(cfg, "", "  ")
	if err != nil {
		return
	}
	os.WriteFile(path, out, 0600)
}

// Oeffnet denselben Einrichtungsdialog wie beim Installieren (Auftrag
// "Modularer Listener": "spaeter aendern lassen... Startmenue-Eintrag
// ... oder ein einfaches Symbol im Infobereich" - hier beides: der
// Startmenue-Eintrag UND dieser Menuepunkt starten denselben Setup.exe
// mit /SETTINGS, siehe installer.nsi). Der Installer legt dafuer beim
// Installieren eine eigene Kopie seiner selbst im Programmordner ab.
// Existiert sie nicht (z.B. eine sehr alte Installation vor diesem
// Auftrag), wird stattdessen einfach der Konfigurationsordner geoeffnet
// und eine Protokollzeile geschrieben - kein Absturz.
// shellExecuteRunAs startet ein Programm MIT Admin-Elevation (UAC-
// Abfrage) - normales exec.Command()/.Start() nutzt CreateProcess, das
// NIE automatisch erhoeht; der Versuch, eine Admin-exe so zu starten,
// schlaegt mit "erfordert erhoehte Rechte" fehl, OHNE dass je ein UAC-
// Fenster erscheint (live bei Alex am 10.11. beobachtet - Setup.exe
// selbst braucht Admin fuer die Firewall-Regel, main.go laeuft aber
// bewusst OHNE Admin). ShellExecuteW mit dem "runas"-Verb ist der
// dokumentierte Weg, das UAC-Fenster korrekt auszuloesen.
func shellExecuteRunAs(path string, args string) error {
	shell32 := syscall.NewLazyDLL("shell32.dll")
	shellExecuteW := shell32.NewProc("ShellExecuteW")
	verbPtr, err := syscall.UTF16PtrFromString("runas")
	if err != nil {
		return err
	}
	pathPtr, err := syscall.UTF16PtrFromString(path)
	if err != nil {
		return err
	}
	argsPtr, err := syscall.UTF16PtrFromString(args)
	if err != nil {
		return err
	}
	const swShowNormal = 1
	ret, _, _ := shellExecuteW.Call(0, uintptr(unsafe.Pointer(verbPtr)), uintptr(unsafe.Pointer(pathPtr)),
		uintptr(unsafe.Pointer(argsPtr)), 0, swShowNormal)
	// ShellExecuteW gibt bei Erfolg einen Wert > 32 zurueck (Win32-
	// Eigenheit aus 16-Bit-Windows-Zeiten, bis heute so dokumentiert).
	if ret <= 32 {
		return fmt.Errorf("ShellExecuteW fehlgeschlagen (Code %d)", ret)
	}
	return nil
}

func openSettings() {
	exePath, err := os.Executable()
	if err != nil {
		exec.Command("explorer.exe", filepath.Dir(configPath())).Start()
		return
	}
	setupPath := filepath.Join(filepath.Dir(exePath), "Setup.exe")
	if _, err := os.Stat(setupPath); err != nil {
		log.Println("Einstellungen: Setup.exe nicht gefunden (alte Installation?) - oeffne Konfigurationsordner stattdessen")
		exec.Command("explorer.exe", filepath.Dir(configPath())).Start()
		return
	}
	if err := shellExecuteRunAs(setupPath, "/SETTINGS"); err != nil {
		log.Printf("Einstellungen konnten nicht geoeffnet werden: %v", err)
	}
}

func reloadToken() {
	cfgMu.Lock()
	cfg = loadConfig()
	tok := cfg.Token
	cfgMu.Unlock()
	updateStatus()
	if tok == "" {
		log.Println("Token neu geladen: LEER - Befehle werden weiter ignoriert")
	} else {
		log.Println("Token neu geladen")
	}
}

func updateStatus() {
	cfgMu.Lock()
	set := cfg.Token != ""
	name := cfg.DeviceName
	port := effectivePort(cfg)
	cfgMu.Unlock()
	portErrMu.Lock()
	pErr := portErr
	portErrMu.Unlock()
	if mStatus == nil {
		return
	}
	if pErr != "" {
		mStatus.SetTitle("Status: " + pErr)
		return
	}
	namePart := ""
	if name != "" {
		namePart = " (" + name + ")"
	}
	if set {
		mStatus.SetTitle(fmt.Sprintf("Status: Token gesetzt, bereit auf Port %d%s", port, namePart))
	} else {
		mStatus.SetTitle("Status: KEIN Token - siehe docs/TOKEN.md")
	}
}

func allowResync() {
	resyncMu.Lock()
	resyncUntilMonoSec = monoSeconds() + resyncWindowS
	resyncMu.Unlock()
	log.Println("Resync-Fenster fuer 60s geoeffnet")
}

func runShutdown(src string) {
	log.Printf("Gueltiger Shutdown-Befehl von %s erhalten - fahre herunter", src)
	// Auftrag "Monitor-Umschaltung" - VOR dem eigentlichen Shutdown, da
	// Windows danach keine DDC-Befehle mehr senden kann. Blockiert das
	// Herunterfahren NIE laenger als monitorDdcTimeout, egal ob DDC
	// gelingt, fehlschlaegt oder haengt (siehe switchMonitorInputBeforeShutdown()).
	// cfg wird unter cfgMu kopiert statt direkt gelesen - gleiches
	// Schutzmuster wie an den anderen cfg-Lesestellen in diesem Codex.
	cfgMu.Lock()
	cfgSnapshot := cfg
	cfgMu.Unlock()
	switchMonitorInputBeforeShutdown(cfgSnapshot)
	time.Sleep(1500 * time.Millisecond)
	// /s herunterfahren, /t 0 sofort, /f erzwingt das Schliessen
	// laufender Programme (sonst kann ein Dialog den Shutdown blockieren).
	cmd := exec.Command("shutdown", "/s", "/t", "0", "/f")
	cmd.Run()
}

// Auftrag "Monitor-Umschaltung" - DDC/CI ueber die in Windows eingebaute
// dxva2.dll (kein Drittanbieter-Tool, gleiches Prinzip wie
// tools/ddc-test/windows-scripts/ddc-umschalten.ps1, hier in Go statt
// PowerShell). Schreibt NUR VCP-Code 0x60 und NUR an den ueber
// MonitorScreenNameSubstring gefundenen Monitor.
var (
	user32Dll = syscall.NewLazyDLL("user32.dll")
	dxva2Dll  = syscall.NewLazyDLL("dxva2.dll")

	procEnumDisplayMonitors                     = user32Dll.NewProc("EnumDisplayMonitors")
	procGetNumberOfPhysicalMonitorsFromHMONITOR = dxva2Dll.NewProc("GetNumberOfPhysicalMonitorsFromHMONITOR")
	procGetPhysicalMonitorsFromHMONITOR         = dxva2Dll.NewProc("GetPhysicalMonitorsFromHMONITOR")
	procDestroyPhysicalMonitors                 = dxva2Dll.NewProc("DestroyPhysicalMonitors")
	procSetVCPFeature                           = dxva2Dll.NewProc("SetVCPFeature")
)

const monitorDdcTimeout = 3 * time.Second
const vcpInputSelectCode = 0x60

// physicalMonitor spiegelt die Windows-Struktur PHYSICAL_MONITOR
// (dxva2.h): ein Handle plus eine feste 128-WCHAR-Beschreibung.
type physicalMonitor struct {
	Handle      syscall.Handle
	Description [128]uint16
}

type rect struct{ Left, Top, Right, Bottom int32 }

// findMonitorHandle sucht per EnumDisplayMonitors+GetPhysicalMonitors-
// FromHMONITOR den ERSTEN physischen Monitor, dessen Windows-Beschreibung
// nameSubstring (gross/klein-unabhaengig) enthaelt. Gibt alle NICHT
// passenden Handles sofort wieder frei (DestroyPhysicalMonitors) - nur
// das gefundene (falls welches) bleibt offen, bis der Aufrufer fertig ist.
func findMonitorHandle(nameSubstring string) (syscall.Handle, string, bool) {
	var foundHandle syscall.Handle
	var foundName string
	foundOk := false
	needle := strings.ToLower(nameSubstring)

	cb := syscall.NewCallback(func(hMonitor syscall.Handle, _ syscall.Handle, _ *rect, _ uintptr) uintptr {
		var count uint32
		ret, _, _ := procGetNumberOfPhysicalMonitorsFromHMONITOR.Call(uintptr(hMonitor), uintptr(unsafe.Pointer(&count)))
		if ret == 0 || count == 0 {
			return 1 // Aufzaehlung fortsetzen
		}
		monitors := make([]physicalMonitor, count)
		ret, _, _ = procGetPhysicalMonitorsFromHMONITOR.Call(uintptr(hMonitor), uintptr(count), uintptr(unsafe.Pointer(&monitors[0])))
		if ret == 0 {
			return 1
		}
		for i := range monitors {
			desc := syscall.UTF16ToString(monitors[i].Description[:])
			if !foundOk && strings.Contains(strings.ToLower(desc), needle) {
				foundHandle = monitors[i].Handle
				foundName = desc
				foundOk = true
			} else {
				destroyArr := monitors[i : i+1]
				procDestroyPhysicalMonitors.Call(1, uintptr(unsafe.Pointer(&destroyArr[0])))
			}
		}
		return 1
	})
	procEnumDisplayMonitors.Call(0, 0, cb, 0)
	return foundHandle, foundName, foundOk
}

func switchMonitorInputBeforeShutdown(c config) {
	if !c.MonitorSwitchEnabled {
		return
	}
	if strings.TrimSpace(c.MonitorScreenNameSubstring) == "" {
		log.Println("Monitor-Umschaltung: kein Monitor ausgewaehlt - nichts getan")
		return
	}

	done := make(chan bool, 1)
	go func() {
		handle, name, ok := findMonitorHandle(c.MonitorScreenNameSubstring)
		if !ok {
			log.Printf("Monitor-Umschaltung: kein Monitor gefunden, der %q enthaelt - nichts getan", c.MonitorScreenNameSubstring)
			done <- false
			return
		}
		ret, _, _ := procSetVCPFeature.Call(uintptr(handle), uintptr(vcpInputSelectCode), uintptr(c.MonitorVcpValue))
		destroyArr := []physicalMonitor{{Handle: handle}}
		procDestroyPhysicalMonitors.Call(1, uintptr(unsafe.Pointer(&destroyArr[0])))
		if ret == 0 {
			log.Printf("Monitor-Umschaltung: DDC-Befehl an %q (Rohwert %d) fehlgeschlagen", name, c.MonitorVcpValue)
			done <- false
			return
		}
		log.Printf("Monitor-Umschaltung: Eingang auf Rohwert %d gesetzt (%q)", c.MonitorVcpValue, name)
		done <- true
	}()
	select {
	case <-done:
	case <-time.After(monitorDdcTimeout):
		log.Printf("Monitor-Umschaltung: Zeitueberschreitung nach %s - fahre trotzdem herunter", monitorDdcTimeout)
	}
}

// computeHmacHex - identische Berechnung wie cyd/src/cmd_auth.cpp
// (hmacHex()) und mac/fan_boost.py (_compute_hmac_hex()): HMAC-SHA256,
// erste 8 Byte (16 Hex-Zeichen).
func computeHmacHex(token, message string) string {
	mac := hmac.New(sha256.New, []byte(token))
	mac.Write([]byte(message))
	full := mac.Sum(nil)
	return hex.EncodeToString(full[:hmacHexLen/2])
}

// verifySigned prueft NUR die Signatur (Format + HMAC), NICHT die
// Zaehler-Reihenfolge - das entscheidet der Aufrufer (RESYNC hat
// andere Regeln). Gibt (nachricht, zaehler, true) bei Erfolg zurueck.
func verifySigned(line, token string) (string, uint32, bool) {
	idx2 := strings.LastIndex(line, " ")
	if idx2 == -1 {
		return "", 0, false
	}
	hmacHex := strings.ToLower(line[idx2+1:])
	rest := line[:idx2]

	idx1 := strings.LastIndex(rest, " ")
	if idx1 == -1 {
		return "", 0, false
	}
	counterStr := rest[idx1+1:]
	message := rest[:idx1]

	counter64, err := strconv.ParseUint(counterStr, 10, 32)
	if err != nil {
		return "", 0, false
	}
	if len(hmacHex) != hmacHexLen {
		return "", 0, false
	}
	for _, c := range hmacHex {
		if (c < '0' || c > '9') && (c < 'a' || c > 'f') {
			return "", 0, false
		}
	}

	expected := computeHmacHex(token, message+" "+counterStr)
	if !hmac.Equal([]byte(expected), []byte(hmacHex)) {
		return "", 0, false
	}
	return message, uint32(counter64), true
}

// Protokolliert jetzt auch abgelehnte Befehle (Auftrag "Modularer
// Listener": "Ein falscher Schluessel fuehrt zu einer Logzeile" -
// vorher war JEDE Ablehnung komplett still, liess sich von "Paket kam
// nie an" nicht unterscheiden, siehe Chat mit Alex 10.11.). NIE den
// Token selbst loggen, nur die Art der Ablehnung.
// "src" ist NUR die Quelladresse (IP:Port) des empfangenen Pakets, fuer
// die Protokollierung (Auftrag "Broadcast-Regression pruefen", 10.11. -
// Alex soll am Log sehen koennen, ob ueberhaupt ein Paket ankommt, auch
// wenn es ungueltig ist). Der Tokenwert selbst wird NIE geloggt.
func handleLine(line string, src string) {
	cfgMu.Lock()
	token := cfg.Token
	lastCounter := cfg.LastCmdCounter
	allowLegacy := cfg.AllowLegacyCommands
	cfgMu.Unlock()
	if token == "" {
		log.Printf("Befehl von %s empfangen, aber kein Token gesetzt - ignoriert", src)
		return
	}

	message, counter, ok := verifySigned(line, token)
	if ok {
		if message == "RESYNC" {
			resyncMu.Lock()
			open := monoSeconds() <= resyncUntilMonoSec
			resyncUntilMonoSec = 0 // einmalig, Fenster danach sofort schliessen
			resyncMu.Unlock()
			if !open {
				log.Printf("RESYNC-Befehl von %s erhalten, aber kein Resync-Fenster offen - ignoriert", src)
				return
			}
			cfgMu.Lock()
			cfg.LastCmdCounter = counter
			saveConfigLocked()
			cfgMu.Unlock()
			log.Printf("RESYNC von %s akzeptiert - Zaehler neu auf %d gesetzt", src, counter)
			return
		}
		if counter <= lastCounter {
			log.Printf("Befehl '%s' von %s verworfen: Zaehler %d nicht neuer als zuletzt akzeptierter Zaehler %d (Wiederholung oder veraltet)", message, src, counter, lastCounter)
			return
		}
		cfgMu.Lock()
		cfg.LastCmdCounter = counter
		saveConfigLocked()
		cfgMu.Unlock()
		if message == "SHUTDOWN" {
			runShutdown(src)
		}
		return
	}

	// Kein gueltig signierter Befehl - nur bei explizit aktiviertem
	// Legacy-Schalter (Standard AUS) auf das alte, unsignierte Format
	// zurueckfallen (KEIN Wiederholungsschutz dabei!).
	if allowLegacy && line == "SHUTDOWN "+token {
		log.Printf("Altes, unsigniertes SHUTDOWN-Format von %s akzeptiert (Legacy-Schalter aktiv)", src)
		runShutdown(src)
		return
	}
	log.Printf("Befehl mit ungueltiger Signatur von %s verworfen (falscher Schluessel oder kein bekanntes Format)", src)
}

// Haengt sich nie mehr mit log.Fatalf() auf, wenn der Port belegt ist
// (Auftrag "Modularer Listener": "Der Listener... stuerzt nicht ab") -
// das Taskleisten-Symbol bleibt sichtbar/bedienbar (z.B. um die
// Konfiguration zu oeffnen und den Port zu aendern), der Status zeigt
// den Fehler, und alle paar Sekunden wird ein neuer Bindungsversuch
// unternommen (z.B. falls das andere Programm, das den Port belegt,
// zwischenzeitlich beendet wird).
func listen() {
	for {
		cfgMu.Lock()
		port := effectivePort(cfg)
		cfgMu.Unlock()

		addr := net.UDPAddr{Port: port, IP: net.IPv4zero}
		conn, err := net.ListenUDP("udp", &addr)
		if err != nil {
			log.Printf("Port %d konnte nicht geoeffnet werden (vermutlich belegt): %v - neuer Versuch in 10s", port, err)
			setPortError(fmt.Sprintf("Port %d belegt - siehe Protokoll", port))
			time.Sleep(10 * time.Second)
			continue
		}
		log.Printf("Lauscht auf UDP-Port %d", port)
		setPortError("")

		buf := make([]byte, 256)
		for {
			n, raddr, err := conn.ReadFromUDP(buf)
			if err != nil {
				time.Sleep(time.Second)
				continue
			}
			line := strings.TrimSpace(string(buf[:n]))
			src := "?"
			if raddr != nil {
				src = raddr.String()
			}
			handleLine(line, src)
		}
	}
}

func onReady() {
	systray.SetIcon(iconData)
	systray.SetTitle("")
	systray.SetTooltip("Eiswolfs iBoard Listener")

	mStatus = systray.AddMenuItem("Status: ...", "")
	mStatus.Disable()
	systray.AddSeparator()
	mReload := systray.AddMenuItem("Token neu laden", "Liest die config.json erneut ein, ohne neu zu starten")
	mSettings := systray.AddMenuItem("Einstellungen...", "Öffnet denselben Einrichtungsdialog wie beim Installieren")
	mResync := systray.AddMenuItem("Zaehler-Resync zulassen (60s)", "Fuer den Fall, dass das CYD seinen Zaehler verloren hat")
	mOpenConfig := systray.AddMenuItem("Konfiguration öffnen", "Öffnet den Ordner mit der config.json")
	mOpenLog := systray.AddMenuItem("Protokoll öffnen", "Öffnet die listener.log")
	systray.AddSeparator()
	mLegacy = systray.AddMenuItem("Alte, unsichere Befehle zulassen", "Nur fuer eine kurze Uebergangszeit gedacht")
	systray.AddSeparator()
	mQuit := systray.AddMenuItem("Beenden", "Beendet den Listener")

	reloadToken()
	cfgMu.Lock()
	if cfg.AllowLegacyCommands {
		mLegacy.Check()
	}
	cfgMu.Unlock()

	go func() {
		for {
			select {
			case <-mReload.ClickedCh:
				reloadToken()
			case <-mSettings.ClickedCh:
				openSettings()
			case <-mResync.ClickedCh:
				allowResync()
			case <-mOpenConfig.ClickedCh:
				exec.Command("explorer.exe", filepath.Dir(configPath())).Start()
			case <-mOpenLog.ClickedCh:
				exec.Command("notepad.exe", filepath.Join(filepath.Dir(configPath()), "listener.log")).Start()
			case <-mLegacy.ClickedCh:
				cfgMu.Lock()
				cfg.AllowLegacyCommands = !cfg.AllowLegacyCommands
				newVal := cfg.AllowLegacyCommands
				saveConfigLocked()
				cfgMu.Unlock()
				if newVal {
					mLegacy.Check()
				} else {
					mLegacy.Uncheck()
				}
			case <-mQuit.ClickedCh:
				systray.Quit()
				return
			}
		}
	}()

	go listen()
}

func onExit() {
	log.Println("Eiswolfs iBoard Windows-Listener beendet")
}

// listAllMonitorDescriptions gibt die Windows-Beschreibung JEDES
// gefundenen physischen Monitors zurueck (anders als findMonitorHandle(),
// das nur den ERSTEN passenden sucht) - fuer den Installer-Dialog
// (Auftrag "Listener-Fenster"), der so eine echte Auswahlliste zeigen
// kann statt eines reinen Freitextfelds.
func listAllMonitorDescriptions() []string {
	var names []string
	cb := syscall.NewCallback(func(hMonitor syscall.Handle, _ syscall.Handle, _ *rect, _ uintptr) uintptr {
		var count uint32
		ret, _, _ := procGetNumberOfPhysicalMonitorsFromHMONITOR.Call(uintptr(hMonitor), uintptr(unsafe.Pointer(&count)))
		if ret == 0 || count == 0 {
			return 1
		}
		monitors := make([]physicalMonitor, count)
		ret, _, _ = procGetPhysicalMonitorsFromHMONITOR.Call(uintptr(hMonitor), uintptr(count), uintptr(unsafe.Pointer(&monitors[0])))
		if ret == 0 {
			return 1
		}
		for i := range monitors {
			names = append(names, syscall.UTF16ToString(monitors[i].Description[:]))
			destroyArr := monitors[i : i+1]
			procDestroyPhysicalMonitors.Call(1, uintptr(unsafe.Pointer(&destroyArr[0])))
		}
		return 1
	})
	procEnumDisplayMonitors.Call(0, 0, cb, 0)
	return names
}

func main() {
	// Auftrag "Listener-Fenster" - Sonderaufruf NUR fuer den Installer-
	// Dialog (nsExec::ExecToStack), gibt je einen erkannten Monitornamen
	// pro Zeile aus und beendet sich sofort, OHNE Taskleisten-Symbol/
	// Log/Mehrfachstart-Sperre anzufassen - rein lesend, keine
	// Nebenwirkung auf eine evtl. schon laufende normale Instanz.
	if len(os.Args) > 1 && os.Args[1] == "--list-monitors" {
		for _, name := range listAllMonitorDescriptions() {
			fmt.Println(name)
		}
		return
	}

	logDir := filepath.Join(os.Getenv("APPDATA"), "EiswolfsIBoard")
	os.MkdirAll(logDir, 0700)
	if f, err := os.OpenFile(filepath.Join(logDir, "listener.log"), os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0600); err == nil {
		log.SetOutput(f)
	}
	fmt.Fprintln(os.Stderr, "Eiswolfs iBoard Windows-Listener gestartet")
	log.Println("Eiswolfs iBoard Windows-Listener gestartet")

	if !acquireSingleInstanceLock() {
		log.Println("Laeuft bereits in einer anderen Instanz - beende mich leise.")
		return
	}

	systray.Run(onReady, onExit)
}
