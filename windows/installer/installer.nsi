; Eiswolfs iBoard Windows-Listener - Installer (Auftrag "Windows-Laptop
; herunterfahren"). Installiert die Hintergrund-exe (mit Taskleisten-
; Symbol, Auftrag "das muss man finden koennen"), legt eine Verknuepfung
; im Startmenue UND im Autostart-Ordner an und startet sie direkt.
; Gebaut mit: makensis installer.nsi (auch von macOS aus moeglich).

!define APP_NAME "Eiswolfs iBoard Listener"
!define APP_DIR "$LOCALAPPDATA\EiswolfsIBoardWin"
!define EXE_NAME "EiswolfsIBoardListener.exe"

Name "${APP_NAME}"
OutFile "EiswolfsIBoardListener-Setup.exe"
InstallDir "${APP_DIR}"
RequestExecutionLevel user
Icon "icon.ico"
UninstallIcon "icon.ico"

Page directory
Page instfiles

Section "Install"
    SetOutPath "$INSTDIR"
    File "EiswolfsIBoardListener.exe"
    File "icon.ico"

    ; Startmenue-Eintrag (Auftrag "ein Eintrag bei Start/Programme") -
    ; eigene Programmgruppe mit Start- und Uninstall-Verknuepfung, damit
    ; man das Programm auch findet, ohne den Installationsordner zu kennen.
    CreateDirectory "$SMPROGRAMS\Eiswolfs iBoard"
    CreateShortcut "$SMPROGRAMS\Eiswolfs iBoard\${APP_NAME}.lnk" "$INSTDIR\${EXE_NAME}" "" "$INSTDIR\icon.ico"
    CreateShortcut "$SMPROGRAMS\Eiswolfs iBoard\Deinstallieren.lnk" "$INSTDIR\Uninstall.exe"

    ; Autostart-Verknuepfung - startet die exe bei jeder Anmeldung,
    ; braucht keinen Windows-Dienst und keine Admin-Rechte.
    CreateDirectory "$SMPROGRAMS\Startup"
    CreateShortcut "$SMSTARTUP\${APP_NAME}.lnk" "$INSTDIR\${EXE_NAME}" "" "$INSTDIR\icon.ico"

    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Gleich starten, nicht erst beim naechsten Login warten - taucht
    ; danach sofort als Symbol in der Taskleiste auf.
    Exec '"$INSTDIR\${EXE_NAME}"'

    MessageBox MB_OK "Eiswolfs iBoard Listener ist installiert und läuft jetzt im Hintergrund (Symbol unten rechts in der Taskleiste, ggf. unter dem Pfeil 'Ausgeblendete Symbole einblenden').$\r$\n$\r$\nÜber das Symbol: Rechtsklick → Konfiguration öffnen, dort die config.json bearbeiten und denselben Token eintragen, den auch CYD und Mac-App benutzen (siehe docs/TOKEN.md im Projekt). Danach im selben Menü auf 'Token neu laden' klicken - kein Neustart nötig.$\r$\n$\r$\nOhne Token ignoriert der Listener jeden Befehl - das ist Absicht."
SectionEnd

Section "Uninstall"
    Delete "$SMPROGRAMS\Eiswolfs iBoard\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\Eiswolfs iBoard\Deinstallieren.lnk"
    RMDir "$SMPROGRAMS\Eiswolfs iBoard"
    Delete "$SMSTARTUP\${APP_NAME}.lnk"
    Delete "$INSTDIR\${EXE_NAME}"
    Delete "$INSTDIR\icon.ico"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"
SectionEnd
