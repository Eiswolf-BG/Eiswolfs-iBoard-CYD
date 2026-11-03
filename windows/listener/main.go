// Eiswolfs iBoard - Windows-Hintergrund-Listener (Auftrag "Windows-Laptop
// herunterfahren"). Gegenstueck zur Mac-App: lauscht per UDP auf
// Config::WIN_SHUTDOWN_UDP_PORT (siehe cyd/src/config.h, Port 4212) auf
// "SHUTDOWN <token>" vom CYD und loest bei gueltigem Token Windows'
// eigenen Shutdown aus.
//
// Taskleisten-Symbol + Menue (Auftrag "das muss man finden koennen") -
// zeigt den Token-Status, erlaubt ein Neuladen der config.json OHNE
// Prozess-Neustart (loest Alex' eigentliches Problem: Token ersetzt,
// aber Listener kannte ihn noch nicht) und einen "Beenden"-Knopf.
//
// Der Token steht NIE im Quellcode - er wird beim ersten Start in
// %APPDATA%\EiswolfsIBoard\config.json mit einem leeren Platzhalter
// angelegt; Alex traegt denselben Token ein, den auch die Mac-App und
// das CYD benutzen (siehe docs/TOKEN.md). Ohne gesetzten Token
// ignoriert der Listener jeden Befehl.
package main

import (
	_ "embed"
	"encoding/json"
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/getlantern/systray"
)

const udpPort = 4212

//go:embed icon.ico
var iconData []byte

type config struct {
	Token string `json:"token"`
}

var (
	tokenMu     sync.RWMutex
	currentTok  string
	mStatus     *systray.MenuItem
)

func configPath() string {
	appData := os.Getenv("APPDATA")
	if appData == "" {
		appData = "."
	}
	return filepath.Join(appData, "EiswolfsIBoard", "config.json")
}

// Legt die Config-Datei mit leerem Token an, falls sie noch nicht
// existiert - ueberschreibt eine vorhandene Datei NIE (gleiches Prinzip
// wie mac/config.py::_load()).
func loadConfig() config {
	path := configPath()
	data, err := os.ReadFile(path)
	if err == nil {
		var cfg config
		if json.Unmarshal(data, &cfg) == nil {
			return cfg
		}
	}

	os.MkdirAll(filepath.Dir(path), 0700)
	cfg := config{Token: ""}
	if out, err := json.MarshalIndent(cfg, "", "  "); err == nil {
		os.WriteFile(path, out, 0600)
	}
	return cfg
}

func reloadToken() {
	cfg := loadConfig()
	tokenMu.Lock()
	currentTok = cfg.Token
	tokenMu.Unlock()
	updateStatus()
	if cfg.Token == "" {
		log.Println("Token neu geladen: LEER - Befehle werden weiter ignoriert")
	} else {
		log.Println("Token neu geladen")
	}
}

func updateStatus() {
	tokenMu.RLock()
	set := currentTok != ""
	tokenMu.RUnlock()
	if mStatus == nil {
		return
	}
	if set {
		mStatus.SetTitle("Status: Token gesetzt, bereit")
	} else {
		mStatus.SetTitle("Status: KEIN Token - siehe docs/TOKEN.md")
	}
}

func runShutdown() {
	log.Println("Gueltiger Shutdown-Befehl erhalten - fahre herunter")
	// /s herunterfahren, /t 0 sofort, /f erzwingt das Schliessen
	// laufender Programme (sonst kann ein Dialog den Shutdown blockieren).
	cmd := exec.Command("shutdown", "/s", "/t", "0", "/f")
	cmd.Run()
}

func listen() {
	addr := net.UDPAddr{Port: udpPort, IP: net.IPv4zero}
	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		log.Fatalf("Konnte Port %d nicht oeffnen: %v", udpPort, err)
	}
	defer conn.Close()

	buf := make([]byte, 256)
	for {
		n, _, err := conn.ReadFromUDP(buf)
		if err != nil {
			time.Sleep(time.Second)
			continue
		}
		line := strings.TrimSpace(string(buf[:n]))
		tokenMu.RLock()
		tok := currentTok
		tokenMu.RUnlock()
		if tok == "" {
			continue
		}
		if line == "SHUTDOWN "+tok {
			runShutdown()
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
	mOpenConfig := systray.AddMenuItem("Konfiguration öffnen", "Öffnet den Ordner mit der config.json")
	mOpenLog := systray.AddMenuItem("Protokoll öffnen", "Öffnet die listener.log")
	systray.AddSeparator()
	mQuit := systray.AddMenuItem("Beenden", "Beendet den Listener")

	reloadToken()

	go func() {
		for {
			select {
			case <-mReload.ClickedCh:
				reloadToken()
			case <-mOpenConfig.ClickedCh:
				exec.Command("explorer.exe", filepath.Dir(configPath())).Start()
			case <-mOpenLog.ClickedCh:
				exec.Command("notepad.exe", filepath.Join(filepath.Dir(configPath()), "listener.log")).Start()
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

func main() {
	logDir := filepath.Join(os.Getenv("APPDATA"), "EiswolfsIBoard")
	os.MkdirAll(logDir, 0700)
	if f, err := os.OpenFile(filepath.Join(logDir, "listener.log"), os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0600); err == nil {
		log.SetOutput(f)
	}
	fmt.Fprintln(os.Stderr, "Eiswolfs iBoard Windows-Listener gestartet")
	log.Println("Eiswolfs iBoard Windows-Listener gestartet")

	systray.Run(onReady, onExit)
}
