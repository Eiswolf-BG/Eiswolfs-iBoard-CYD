; Eiswolfs iBoard Windows-Listener - Installer (Auftrag "Windows-Laptop
; herunterfahren", seit Auftrag "Modularer Listener" um eine eigene
; Einrichtungsseite, Firewall-Regel und einen Einstellungen-Wiederaufruf
; erweitert). Installiert die Hintergrund-exe (mit Taskleisten-Symbol),
; fragt Geraetename/Port/Schluessel/Autostart ab, legt Start-/Autostart-
; Verknuepfungen an und startet den Listener direkt.
; Gebaut mit: makensis installer.nsi (auch von macOS aus moeglich).
;
; Admin-Rechte (Auftrag, nach Ruecksprache mit Alex): noetig fuer die
; automatische Windows-Firewall-Regel (netsh laeuft sonst nicht). Zeigt
; beim Start einmalig ein UAC-Fenster - vorher kam das Setup ohne aus.

!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "StrFunc.nsh"
!include "nsDialogs.nsh"
${Using:StrFunc} StrStr

!define APP_NAME "Eiswolfs iBoard Listener"
!define APP_DIR "$LOCALAPPDATA\EiswolfsIBoardWin"
!define EXE_NAME "EiswolfsIBoardListener.exe"
!define SETUP_COPY_NAME "Setup.exe"
!define CONFIG_DIR "$APPDATA\EiswolfsIBoard"
!define CONFIG_FILE "${CONFIG_DIR}\config.json"
!define FW_RULE_NAME "Eiswolfs iBoard Listener"
; Muss zu Config::WIN_SHUTDOWN_UDP_PORT (cyd/src/config.h) passen - nur
; der VORGABEWERT im Einrichtungsdialog, siehe dort.
!define DEFAULT_PORT "4212"

Name "${APP_NAME}"
OutFile "EiswolfsIBoardListener-Setup.exe"
InstallDir "${APP_DIR}"
RequestExecutionLevel admin
Icon "icon.ico"
UninstallIcon "icon.ico"

Var Dialog
Var hName
Var hPort
Var hToken
Var hAutostart
Var hBrowseBtn
Var hHint
Var hTokenInfo

Var ExistingDeviceName
Var ExistingPort
Var ExistingToken
Var ExistingAutostart
Var ExistingLastCounter
Var ExistingAllowLegacy

; Auftrag "Monitor-Umschaltung" - additiv, gleiches Lese-/Schreibmuster
; wie die bestehenden Existing*/New*-Variablen oben.
Var ExistingMonitorEnabled
Var ExistingMonitorVcpValue
Var ExistingMonitorScreenSubstring
Var NewMonitorEnabledState
Var NewMonitorVcpValue
Var NewMonitorScreenSubstring
Var hMonitorEnabled
Var hMonitorVcpValue
Var hMonitorScreenSubstring
Var hMonitorInputChoice
Var hMonitorNoneFoundLabel
Var MonitorListText
Var MonitorListCount
Var MonitorListTmpExe

Var NewDeviceName
Var NewPort
Var NewToken
Var NewAutostartState

; Auftrag "...Einstellungen spaeter aendern...": beim Aufruf als
; "<Setup.exe> /SETTINGS" (vom Taskleisten-Menuepunkt UND vom
; Startmenue-Eintrag "Eiswolfs iBoard Listener Einstellungen" genutzt,
; siehe main.go::openSettings()) wird NICHTS neu installiert, nur die
; Einrichtungsseite gezeigt und die config.json neu geschrieben -
; Abschnitt "Install" ist dabei bewusst trotzdem idempotent (schreibt
; dieselben Dateien/Verknuepfungen einfach erneut, kein eigener Pfad
; noetig).
Var IsSettingsOnly

Function .onInit
    StrCpy $IsSettingsOnly "0"
    ${GetParameters} $0
    ${GetOptions} $0 "/SETTINGS" $1
    ${IfNot} ${Errors}
        StrCpy $IsSettingsOnly "1"
    ${EndIf}
    ClearErrors
FunctionEnd

; Hilfsfunktionen zum JSON-Einlesen (Auftrag "Modularer Listener") -
; bewusst OHNE den NSIS-Stack als Parameteruebergabe (Push/Pop/Exch mit
; mehreren Werten hatte sich als fehleranfaellig erwiesen: live bei
; Alex am 10.11. beobachtet - der Token kam nach jeder Neueinrichtung
; leer heraus, laut Protokoll/`type config.json` auch tatsaechlich leer
; auf der Platte, nicht nur in der Anzeige. Ursache: die alte Fassung
; dieser Funktionen balancierte Push/Exch/Pop nicht korrekt, liess pro
; Aufruf Stack-Reste zurueck, die sich ueber mehrere Feld-Extraktionen
; hinweg aufsummierten und spaetere Felder - u.a. genau "token" -
; verfaelschten). Jetzt stattdessen ueber feste, eigene Variablen
; (JsonIn*/JsonOut) - weniger "idiomatisch", dafuer nachvollziehbar
; korrekt.
Var JsonInPath
Var JsonInText
Var JsonInPattern
Var JsonOutValue

; Liest eine evtl. vorhandene Datei komplett in EINEN String ein
; (Zeilenumbrueche spielen fuer die nachfolgende Stichwort-Suche keine
; Rolle) - Grundlage fuer "bestehende Installation erkennen, nichts neu
; eingeben muessen" (Auftrag) UND fuer das Auslesen einer vom Nutzer
; ausgewaehlten Datei (siehe OnBrowseToken). Liest aus $JsonInPath,
; schreibt das Ergebnis nach $JsonInText (leer, falls nicht vorhanden/
; nicht lesbar).
Function ReadJsonFile
    StrCpy $JsonInText ""
    ${IfNot} ${FileExists} $JsonInPath
        Return
    ${EndIf}
    Push $0
    Push $1
    FileOpen $0 $JsonInPath r
    ${DoUntil} ${Errors}
        FileRead $0 $1
        StrCpy $JsonInText "$JsonInText$1"
    ${Loop}
    FileClose $0
    ClearErrors
    Pop $1
    Pop $0
FunctionEnd

; Extrahiert den Wert eines in Anfuehrungszeichen stehenden JSON-Feldes
; (z.B. "token": "abc") aus $JsonInText. Sucht nach $JsonInPattern
; (INKLUSIVE des oeffnenden Anfuehrungszeichens, z.B. '"token": "').
; Ergebnis (ohne Anfuehrungszeichen, oder "" falls nicht gefunden) in
; $JsonOutValue.
Function ExtractQuotedField
    StrCpy $JsonOutValue ""
    Push $0
    Push $1
    Push $2
    Push $3

    ${StrStr} $0 $JsonInText $JsonInPattern
    ${If} $0 != ""
        StrLen $1 $JsonInPattern
        StrCpy $0 $0 "" $1
        ${StrStr} $2 $0 '"'
        StrLen $1 $0
        StrLen $3 $2
        IntOp $1 $1 - $3
        StrCpy $JsonOutValue $0 $1
    ${EndIf}

    Pop $3
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

; Wie ExtractQuotedField, aber fuer eine UNQUOTED Zahl (z.B. "port":
; 4212), endet beim naechsten Komma.
Function ExtractNumberField
    StrCpy $JsonOutValue ""
    Push $0
    Push $1
    Push $2
    Push $3

    ${StrStr} $0 $JsonInText $JsonInPattern
    ${If} $0 != ""
        StrLen $1 $JsonInPattern
        StrCpy $0 $0 "" $1
        ${StrStr} $2 $0 ","
        ${If} $2 == ""
            StrCpy $JsonOutValue $0
        ${Else}
            StrLen $1 $0
            StrLen $3 $2
            IntOp $1 $1 - $3
            StrCpy $JsonOutValue $0 $1
        ${EndIf}
    ${EndIf}

    Pop $3
    Pop $2
    Pop $1
    Pop $0
FunctionEnd

Function ReadExistingConfig
    StrCpy $ExistingDeviceName ""
    StrCpy $ExistingPort "${DEFAULT_PORT}"
    StrCpy $ExistingToken ""
    StrCpy $ExistingAutostart "1"
    StrCpy $ExistingLastCounter "0"
    StrCpy $ExistingAllowLegacy "false"
    StrCpy $ExistingMonitorEnabled "false"
    StrCpy $ExistingMonitorVcpValue ""
    StrCpy $ExistingMonitorScreenSubstring ""

    StrCpy $JsonInPath "${CONFIG_FILE}"
    Call ReadJsonFile
    ${If} $JsonInText == ""
        Return
    ${EndIf}

    StrCpy $JsonInPattern '"device_name": "'
    Call ExtractQuotedField
    ${If} $JsonOutValue != ""
        StrCpy $ExistingDeviceName $JsonOutValue
    ${EndIf}

    StrCpy $JsonInPattern '"token": "'
    Call ExtractQuotedField
    ${If} $JsonOutValue != ""
        StrCpy $ExistingToken $JsonOutValue
    ${EndIf}

    StrCpy $JsonInPattern '"port": '
    Call ExtractNumberField
    ${If} $JsonOutValue != ""
        StrCpy $ExistingPort $JsonOutValue
    ${EndIf}

    StrCpy $JsonInPattern '"last_cmd_counter": '
    Call ExtractNumberField
    ${If} $JsonOutValue != ""
        StrCpy $ExistingLastCounter $JsonOutValue
    ${EndIf}

    Push $0
    ${StrStr} $0 $JsonInText '"allow_legacy_commands": true'
    ${If} $0 != ""
        StrCpy $ExistingAllowLegacy "true"
    ${EndIf}

    ${StrStr} $0 $JsonInText '"autostart": false'
    ${If} $0 != ""
        StrCpy $ExistingAutostart "0"
    ${EndIf}

    ; Auftrag "Monitor-Umschaltung" - additiv, Standard AUS/leer oben.
    ${StrStr} $0 $JsonInText '"monitor_switch_enabled": true'
    ${If} $0 != ""
        StrCpy $ExistingMonitorEnabled "true"
    ${EndIf}
    Pop $0

    StrCpy $JsonInPattern '"monitor_vcp_value": '
    Call ExtractNumberField
    ${If} $JsonOutValue != ""
        StrCpy $ExistingMonitorVcpValue $JsonOutValue
    ${EndIf}

    StrCpy $JsonInPattern '"monitor_screen_name_substring": "'
    Call ExtractQuotedField
    ${If} $JsonOutValue != ""
        StrCpy $ExistingMonitorScreenSubstring $JsonOutValue
    ${EndIf}
FunctionEnd

Function OnBrowseToken
    nsDialogs::SelectFileDialog open "" "JSON-Dateien (*.json)|*.json|Alle Dateien (*.*)|*.*"
    Pop $JsonInPath
    ${If} $JsonInPath == error
        Return
    ${EndIf}
    Call ReadJsonFile
    ${If} $JsonInText == ""
        MessageBox MB_OK "Diese Datei konnte nicht gelesen werden."
        Return
    ${EndIf}
    StrCpy $JsonInPattern '"token": "'
    Call ExtractQuotedField
    ${If} $JsonOutValue == ""
        MessageBox MB_OK "In dieser Datei wurde kein 'token'-Feld gefunden."
    ${Else}
        ${NSD_SetText} $hToken $JsonOutValue
    ${EndIf}
FunctionEnd

; Auftrag "Listener-Fenster": Seite 1 (Name/Port/Schluessel/Autostart)
; und Seite 2 (Monitor-Umschaltung) getrennt, weil das klassische
; (Nicht-MUI) NSIS-Dialogfenster eine FESTE, kleine Hoehe hat (ca.
; 140-150u nutzbarer Innenbereich) - alles, was eine einzelne Seite
; vorher darueber hinaus zeichnete, lag schlicht ausserhalb des
; sichtbaren Fensters (bestaetigt durch Alex' Foto: schon der alte
; Hinweistext war unten abgeschnitten, die neuen Monitor-Felder waren
; komplett unsichtbar). Jede der beiden Seiten bleibt jetzt deutlich
; unter dieser Grenze.
Function SettingsPage1Show
    Call ReadExistingConfig

    nsDialogs::Create 1018
    Pop $Dialog
    ${If} $Dialog == error
        Abort
    ${EndIf}

    ${NSD_CreateLabel} 0 0 100% 10u "Name für dieses Gerät (frei wählbar, nur zur Anzeige, z.B. der Rechnername):"
    Pop $0
    ${NSD_CreateText} 0 11u 100% 12u $ExistingDeviceName
    Pop $hName

    ${NSD_CreateLabel} 0 28u 100% 10u "Port (Vorgabe ${DEFAULT_PORT} - nur ändern, wenn dieser auf diesem Rechner belegt ist):"
    Pop $0
    ${NSD_CreateText} 0 39u 35% 12u $ExistingPort
    Pop $hPort

    ${NSD_CreateLabel} 0 56u 100% 10u "Geheimer Schlüssel (muss identisch mit dem im CYD/in der Mac-App sein):"
    Pop $0
    ; Auftrag "Listener-Fenster", Schritt 3: NIE den vorhandenen
    ; Schluessel im Klartext anzeigen - das Feld bleibt beim Oeffnen
    ; IMMER leer, unabhaengig davon, ob/welcher Schluessel gespeichert
    ; ist. Ein leer gelassenes Feld ueberschreibt den vorhandenen
    ; Schluessel trotzdem nicht (siehe SettingsPage1Leave unten,
    ; unveraendert).
    ${NSD_CreateText} 0 67u 64% 12u ""
    Pop $hToken
    ${NSD_CreateButton} 66% 67u 34% 12u "Aus Datei wählen..."
    Pop $hBrowseBtn
    ${NSD_OnClick} $hBrowseBtn OnBrowseToken

    ; Dauerhafte Anzeige (nicht nur Debug), ob ein bestehender Schluessel
    ; erkannt wurde (Auftrag: "Alex' bestehender Acer... muss OHNE
    ; Neueingabe weiter funktionieren" - macht sichtbar, OB das gerade
    ; zutrifft, statt es stillschweigend anzunehmen; hat beim Live-Test
    ; mit Alex am 10.11. geholfen, einen echten Bug zu finden, bei dem
    ; der Schluessel beim erneuten Einrichten leer wurde). Zeigt NUR die
    ; Laenge, NIE den Wert selbst.
    StrLen $8 $ExistingToken
    ${If} $8 > 0
        ${NSD_CreateLabel} 0 80u 100% 10u "(Vorhandener Schlüssel erkannt: $8 Zeichen - leer lassen, um ihn zu behalten)"
    ${Else}
        ${NSD_CreateLabel} 0 80u 100% 10u "(Kein vorhandener Schlüssel gefunden - bitte eintragen)"
    ${EndIf}
    Pop $hTokenInfo

    ${NSD_CreateCheckBox} 0 94u 100% 10u "Beim Anmelden automatisch starten"
    Pop $hAutostart
    ${If} $ExistingAutostart == "1"
        ${NSD_Check} $hAutostart
    ${EndIf}

    ${NSD_CreateLabel} 0 107u 100% 18u "Schlüssel von der Mac-App übernehmen oder eintragen/einfügen. Siehe docs/TOKEN.md."
    Pop $hHint

    nsDialogs::Show
FunctionEnd

Function SettingsPage1Leave
    ${NSD_GetText} $hName $NewDeviceName
    ${NSD_GetText} $hPort $NewPort
    ${NSD_GetText} $hToken $NewToken
    ${NSD_GetState} $hAutostart $NewAutostartState

    ${If} $NewDeviceName == ""
        StrCpy $NewDeviceName "Laptop"
    ${EndIf}
    ${If} $NewPort == ""
        StrCpy $NewPort "${DEFAULT_PORT}"
    ${EndIf}
    ; Ein LEERES Token-Feld NIE als "Token loeschen" werten - behaelt
    ; stattdessen den bisherigen Wert (siehe ExistingToken). Verhindert
    ; genau den Fehler, der Alex live passiert ist (10.11.): ein
    ; abgebrochener/uebersprungener Installationsschritt fuehrte
    ; trotzdem zum Schreiben der config.json mit leerem Token und hat
    ; damit den bestehenden, funktionierenden Token geloescht. Wer den
    ; Token wirklich AENDERN will, muss einen neuen Wert eintragen -
    ; blosses Leerlassen loescht nichts mehr. Gilt unveraendert, obwohl
    ; das Feld jetzt beim OEFFNEN immer leer dargestellt wird (Schritt 3).
    ${If} $NewToken == ""
        StrCpy $NewToken $ExistingToken
    ${EndIf}
FunctionEnd

; Auftrag "Listener-Fenster", Schritt 2: liefert die von Windows erkannten
; Monitor-Beschreibungen (eine pro Zeile) ueber den Sonderaufruf
; "<exe> --list-monitors" (main.go) - dafuer wird die im Installer
; eingebettete exe einmalig in $PLUGINSDIR extrahiert (funktioniert so
; sowohl bei einer Neuinstallation, wo $INSTDIR noch leer ist, als auch
; bei einem Update). $PLUGINSDIR wird von NSIS selbst nach Programmende
; aufgeraeumt. Ergebnis in $MonitorListText (zeilenweise, per $\n
; getrennt) und $MonitorListCount.
Function DetectMonitors
    StrCpy $MonitorListText ""
    StrCpy $MonitorListCount "0"
    InitPluginsDir
    StrCpy $MonitorListTmpExe "$PLUGINSDIR\monitorlist.exe"
    File "/oname=$MonitorListTmpExe" "EiswolfsIBoardListener.exe"
    nsExec::ExecToStack '"$MonitorListTmpExe" --list-monitors'
    Pop $0
    Pop $1
    ${If} $0 == "0"
        StrCpy $MonitorListText $1
    ${EndIf}
FunctionEnd

Function SettingsPage2Show
    nsDialogs::Create 1018
    Pop $Dialog
    ${If} $Dialog == error
        Abort
    ${EndIf}

    ${NSD_CreateLabel} 0 0 100% 16u "Schaltet den Monitor-Eingang vor dem Herunterfahren automatisch zurück (DDC/CI)."
    Pop $0

    ${NSD_CreateCheckBox} 0 18u 100% 10u "Monitor-Eingang beim Herunterfahren automatisch zurückschalten"
    Pop $hMonitorEnabled
    ${If} $ExistingMonitorEnabled == "true"
        ${NSD_Check} $hMonitorEnabled
    ${EndIf}

    ${NSD_CreateLabel} 0 32u 60% 10u "Eingang (Rohwert unten wird vorbelegt):"
    Pop $0
    ; Hoehe hier ist NICHT nur die geschlossene Zeile, sondern muss auch
    ; den AUFGEKLAPPTEN Zustand mit allen 5 Eintraegen Platz bieten -
    ; zu knapp bemessen (wie urspruenglich 13u) fuehrt dazu, dass Windows
    ; die Liste beim Klick sofort wieder zuklappt ("flackert nur", siehe
    ; Alex' Rueckmeldung) statt sie offen zu halten.
    ${NSD_CreateDropList} 0 43u 100% 70u ""
    Pop $hMonitorInputChoice
    ${NSD_CB_AddString} $hMonitorInputChoice "HDMI 1 (Rohwert 4369)"
    ${NSD_CB_AddString} $hMonitorInputChoice "HDMI 2 (Rohwert 4626)"
    ${NSD_CB_AddString} $hMonitorInputChoice "DisplayPort 1 (Rohwert 3855)"
    ${NSD_CB_AddString} $hMonitorInputChoice "DisplayPort 2 (Rohwert 4112)"
    ${NSD_CB_AddString} $hMonitorInputChoice "Anderer Wert (unten eintragen)"
    ; Win32-DropLists (im Unterschied zur ComboBox darunter) zeigen OHNE
    ; eine explizite Auswahl GAR NICHTS an, auch wenn Eintraege vorhanden
    ; sind - nur das Oeffnen per Klick waere nicht betroffen, der
    ; geschlossene Zustand wirkt dann aber "leer" (genau das von Alex
    ; beobachtete Bild). Deshalb hier IMMER eine Auswahl setzen: passend
    ; zum vorhandenen Rohwert, sonst "Anderer Wert" als sichtbarer
    ; Normalzustand.
    ${If} $ExistingMonitorVcpValue == "4369"
        ${NSD_CB_SelectString} $hMonitorInputChoice "HDMI 1 (Rohwert 4369)"
    ${ElseIf} $ExistingMonitorVcpValue == "4626"
        ${NSD_CB_SelectString} $hMonitorInputChoice "HDMI 2 (Rohwert 4626)"
    ${ElseIf} $ExistingMonitorVcpValue == "3855"
        ${NSD_CB_SelectString} $hMonitorInputChoice "DisplayPort 1 (Rohwert 3855)"
    ${ElseIf} $ExistingMonitorVcpValue == "4112"
        ${NSD_CB_SelectString} $hMonitorInputChoice "DisplayPort 2 (Rohwert 4112)"
    ${Else}
        ${NSD_CB_SelectString} $hMonitorInputChoice "Anderer Wert (unten eintragen)"
    ${EndIf}
    ${NSD_OnChange} $hMonitorInputChoice OnMonitorInputChoiceChange

    ${NSD_CreateLabel} 0 60u 60% 10u "VCP-Rohwert:"
    Pop $0
    ${NSD_CreateText} 0 71u 35% 12u $ExistingMonitorVcpValue
    Pop $hMonitorVcpValue

    ${NSD_CreateLabel} 0 87u 100% 10u "Monitor (Name oder Teil davon) - leer = nichts tun:"
    Pop $0
    ; Gleicher Grund wie beim Eingang-Dropdown oben - Hoehe muss den
    ; aufgeklappten Zustand (bis zu mehrere erkannte Monitore) mit
    ; einschliessen.
    ${NSD_CreateComboBox} 0 98u 100% 60u $ExistingMonitorScreenSubstring
    Pop $hMonitorScreenSubstring
    ; Nochmal explizit setzen (zusaetzlich zum Erzeugen-Parameter oben) -
    ; stellt sicher, dass der Text auch dann steht, wenn das Erzeugen
    ; selbst ihn aus irgendeinem Grund nicht uebernommen haben sollte.
    ${NSD_SetText} $hMonitorScreenSubstring $ExistingMonitorScreenSubstring

    Call DetectMonitors
    ${If} $MonitorListText != ""
        Push $MonitorListText
        Push $MonitorListCount
        ; $MonitorListText zeilenweise in die ComboBox eintragen
        StrCpy $3 $MonitorListText
        ${DoUntil} $3 == ""
            ${StrStr} $4 $3 "$\n"
            ${If} $4 == ""
                StrCpy $5 $3
                StrCpy $3 ""
            ${Else}
                StrLen $6 $3
                StrLen $7 $4
                IntOp $6 $6 - $7
                StrCpy $5 $3 $6
                StrCpy $3 $4 "" 1
            ${EndIf}
            ; CR am Zeilenende (falls vorhanden) abschneiden
            StrCpy $8 $5 1 -1
            ${If} $8 == "$\r"
                StrLen $6 $5
                IntOp $6 $6 - 1
                StrCpy $5 $5 $6
            ${EndIf}
            ${If} $5 != ""
                ${NSD_CB_AddString} $hMonitorScreenSubstring $5
            ${EndIf}
        ${Loop}
        Pop $MonitorListCount
        Pop $MonitorListText
        ${NSD_CreateLabel} 0 113u 100% 16u "Oben stehen die erkannten Monitore - oder frei eintippen."
        Pop $hMonitorNoneFoundLabel
    ${Else}
        ${NSD_CreateLabel} 0 113u 100% 16u "Kein Monitor erkannt - bitte Namen von Hand eintragen."
        Pop $hMonitorNoneFoundLabel
    ${EndIf}

    nsDialogs::Show
FunctionEnd

Function OnMonitorInputChoiceChange
    ${NSD_GetText} $hMonitorInputChoice $0
    ${If} $0 == "HDMI 1 (Rohwert 4369)"
        ${NSD_SetText} $hMonitorVcpValue "4369"
    ${ElseIf} $0 == "HDMI 2 (Rohwert 4626)"
        ${NSD_SetText} $hMonitorVcpValue "4626"
    ${ElseIf} $0 == "DisplayPort 1 (Rohwert 3855)"
        ${NSD_SetText} $hMonitorVcpValue "3855"
    ${ElseIf} $0 == "DisplayPort 2 (Rohwert 4112)"
        ${NSD_SetText} $hMonitorVcpValue "4112"
    ${EndIf}
    ; "Anderer Wert" aendert das Textfeld bewusst NICHT - Alex traegt den
    ; Rohwert dann selbst ein.
FunctionEnd

Function SettingsPage2Leave
    ${NSD_GetState} $hMonitorEnabled $NewMonitorEnabledState
    ${NSD_GetText} $hMonitorVcpValue $NewMonitorVcpValue
    ${NSD_GetText} $hMonitorScreenSubstring $NewMonitorScreenSubstring
    ${If} $NewMonitorVcpValue == ""
        StrCpy $NewMonitorVcpValue "0"
    ${EndIf}
FunctionEnd

Page custom SettingsPage1Show SettingsPage1Leave
Page custom SettingsPage2Show SettingsPage2Leave
Page instfiles

Section "Install"
    ; Laeuft bereits eine (aeltere) Kopie - z.B. bei einem Update oder
    ; einem erneuten "Einstellungen..."-Aufruf (main.go::openSettings()
    ; startet den Listener NACH dem Schreiben der config.json neu, es
    ; kann also schon eine Instanz laufen) - die exe ist dann gesperrt
    ; und File() unten schlaegt sonst mit "Error opening file for
    ; writing" fehl (von Alex live beobachtet, 10.11.). Sauber beenden
    ; und kurz warten, BEVOR ueberschrieben wird.
    nsExec::Exec 'taskkill /IM "${EXE_NAME}" /F'
    Pop $0
    Sleep 500

    SetOutPath "$INSTDIR"
    File "EiswolfsIBoardListener.exe"
    File "icon.ico"
    File "..\..\LICENSE"
    File "..\..\THIRD-PARTY-LICENSES.md"
    SetOutPath "$INSTDIR\licenses"
    File "..\..\licenses\*.*"
    SetOutPath "$INSTDIR"

    ; Eigene Kopie des Setups im Programmordner (Auftrag "Einstellungen
    ; spaeter aendern") - der Startmenue-Eintrag "Einstellungen" UND der
    ; Taskleisten-Menuepunkt (main.go::openSettings()) starten DIESE
    ; Kopie mit /SETTINGS, ohne dass Alex die urspruengliche Setup-Datei
    ; wiederfinden muss.
    CopyFiles /SILENT "$EXEPATH" "$INSTDIR\${SETUP_COPY_NAME}"

    ; config.json schreiben (Auftrag) - last_cmd_counter/allow_legacy_
    ; commands werden aus einer bestehenden Datei UEBERNOMMEN (siehe
    ; ReadExistingConfig, in SettingsPageShow bereits aufgerufen), NIE
    ; zurueckgesetzt - ein Reset wuerde alte, bereits einmal verbrauchte
    ; Befehle wieder wie neu aussehen lassen (Wiederholungsschutz,
    ; siehe docs/TOKEN.md).
    CreateDirectory "${CONFIG_DIR}"
    FileOpen $0 "${CONFIG_FILE}" w
    FileWrite $0 '{$\r$\n'
    FileWrite $0 '  "token": "$NewToken",$\r$\n'
    FileWrite $0 '  "last_cmd_counter": $ExistingLastCounter,$\r$\n'
    FileWrite $0 '  "allow_legacy_commands": $ExistingAllowLegacy,$\r$\n'
    FileWrite $0 '  "device_name": "$NewDeviceName",$\r$\n'
    FileWrite $0 '  "port": $NewPort,$\r$\n'
    ${If} $NewAutostartState == ${BST_CHECKED}
        FileWrite $0 '  "autostart": true,$\r$\n'
    ${Else}
        FileWrite $0 '  "autostart": false,$\r$\n'
    ${EndIf}
    ; Auftrag "Monitor-Umschaltung" - additiv, siehe windows/listener/main.go::config.
    ${If} $NewMonitorEnabledState == ${BST_CHECKED}
        FileWrite $0 '  "monitor_switch_enabled": true,$\r$\n'
    ${Else}
        FileWrite $0 '  "monitor_switch_enabled": false,$\r$\n'
    ${EndIf}
    FileWrite $0 '  "monitor_vcp_value": $NewMonitorVcpValue,$\r$\n'
    FileWrite $0 '  "monitor_screen_name_substring": "$NewMonitorScreenSubstring"$\r$\n'
    FileWrite $0 '}$\r$\n'
    FileClose $0

    ; Startmenue-Eintrag (eigene Programmgruppe) - Start-, Einstellungen-
    ; und Uninstall-Verknuepfung.
    CreateDirectory "$SMPROGRAMS\Eiswolfs iBoard"
    CreateShortcut "$SMPROGRAMS\Eiswolfs iBoard\${APP_NAME}.lnk" "$INSTDIR\${EXE_NAME}" "" "$INSTDIR\icon.ico"
    CreateShortcut "$SMPROGRAMS\Eiswolfs iBoard\${APP_NAME} Einstellungen.lnk" "$INSTDIR\${SETUP_COPY_NAME}" "/SETTINGS" "$INSTDIR\icon.ico"
    CreateShortcut "$SMPROGRAMS\Eiswolfs iBoard\Deinstallieren.lnk" "$INSTDIR\Uninstall.exe"

    ; Autostart-Verknuepfung nur, wenn im Dialog angehakt (Vorgabe: an).
    Delete "$SMSTARTUP\${APP_NAME}.lnk"
    ${If} $NewAutostartState == ${BST_CHECKED}
        CreateShortcut "$SMSTARTUP\${APP_NAME}.lnk" "$INSTDIR\${EXE_NAME}" "" "$INSTDIR\icon.ico"
    ${EndIf}

    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Firewall-Regel automatisch anlegen (Auftrag, nach Ruecksprache -
    ; braucht die Admin-Rechte von RequestExecutionLevel admin oben).
    ; Vorhandene Regel zuerst entfernen (idempotent, kein Duplikat bei
    ; wiederholtem Einrichten/Aendern des Ports).
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="${FW_RULE_NAME}"'
    Pop $0
    nsExec::ExecToLog 'netsh advfirewall firewall add rule name="${FW_RULE_NAME}" dir=in action=allow protocol=UDP localport=$NewPort profile=private program="$INSTDIR\${EXE_NAME}"'
    Pop $0
    ${If} $0 != "0"
        MessageBox MB_OK "Die Firewall-Regel konnte nicht automatisch eingerichtet werden (Fehlercode $0). Falls beim ersten Senden ein Fenster 'Windows Defender Firewall hat Funktionen blockiert' erscheint: nur 'Private Netzwerke' bestätigen, 'Öffentliche Netzwerke' NICHT."
        StrCpy $1 "0"
    ${Else}
        StrCpy $1 "1"
    ${EndIf}

    ; Listener (neu) starten - die eingebaute Mehrfachstart-Sperre
    ; (main.go::acquireSingleInstanceLock()) sorgt dafuer, dass eine
    ; evtl. schon laufende Kopie sich einfach beendet, keine doppelten
    ; Taskleisten-Symbole.
    Exec '"$INSTDIR\${EXE_NAME}"'

    ${If} $IsSettingsOnly == "1"
        MessageBox MB_OK "Einstellungen gespeichert. Der Listener liest sie beim naechsten Neustart automatisch ein (oder jetzt sofort über das Taskleisten-Symbol → 'Token neu laden')."
    ${Else}
        ${If} $1 == "1"
            StrCpy $2 "Die Firewall-Regel wurde automatisch angelegt."
        ${Else}
            StrCpy $2 "Die Firewall-Regel konnte NICHT automatisch angelegt werden - siehe vorherige Meldung."
        ${EndIf}
        MessageBox MB_OK "Eiswolfs iBoard Listener ist installiert, eingerichtet und läuft jetzt im Hintergrund (Symbol unten rechts in der Taskleiste, ggf. unter dem Pfeil 'Ausgeblendete Symbole einblenden'). $2$\r$\n$\r$\nSpäter ändern: Startmenü → Eiswolfs iBoard → '${APP_NAME} Einstellungen', oder über das Taskleisten-Symbol → 'Einstellungen...'."
    ${EndIf}
SectionEnd

Section "Uninstall"
    nsExec::ExecToLog 'netsh advfirewall firewall delete rule name="${FW_RULE_NAME}"'
    Pop $0
    Delete "$SMPROGRAMS\Eiswolfs iBoard\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\Eiswolfs iBoard\${APP_NAME} Einstellungen.lnk"
    Delete "$SMPROGRAMS\Eiswolfs iBoard\Deinstallieren.lnk"
    RMDir "$SMPROGRAMS\Eiswolfs iBoard"
    Delete "$SMSTARTUP\${APP_NAME}.lnk"
    Delete "$INSTDIR\${EXE_NAME}"
    Delete "$INSTDIR\${SETUP_COPY_NAME}"
    Delete "$INSTDIR\icon.ico"
    Delete "$INSTDIR\LICENSE"
    Delete "$INSTDIR\THIRD-PARTY-LICENSES.md"
    RMDir /r "$INSTDIR\licenses"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"
SectionEnd
