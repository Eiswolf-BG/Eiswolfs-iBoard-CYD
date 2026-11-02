"""Ruhemodus-Befehl (Auftrag "Ruhemodus", umgestellt auf echten System-
Sleep nach Alex' ausdruecklichem Wunsch) - loest "pmset sleepnow" aus,
echten, stromsparenden System-Sleep (kein reines Display-/Bildschirm-
sperren mehr, siehe erste Fassung in der Git-Historie dieses Skripts).

Vorgeschichte (siehe CLAUDE.md "Auftrag Ruhemodus: Fortschritt",
Schritt 1): die urspruengliche Fassung simulierte stattdessen Cmd+Ctrl+Q
("System Events"), um GENAU das nachzubilden, was Alex' Fingerabdruck-
taste ohne erkannten Finger tut (per log show bestaetigt: nur eine
Bildschirmsperre, KEIN echter Sleep). Nach einem manuellen Live-Test mit
"pmset sleepnow" hat Alex bestaetigt, dass sich ihr Mac mini per
Fingerabdruck-Tastendruck auch aus echtem System-Sleep zuverlaessig
aufwecken UND entsperren laesst (trotz Bluetooth-Tastatur, nicht
selbstverstaendlich - siehe CLAUDE.md) - deshalb jetzt der
stromsparendere Weg. Braucht weiterhin KEINE Root-Rechte.
"""
import subprocess


def sleep_now():
    """Loest echten System-Sleep aus. Gibt True bei Erfolg zurueck, False
    bei jedem Fehler - wirft nie eine Exception nach aussen."""
    try:
        result = subprocess.run(
            ["pmset", "sleepnow"],
            capture_output=True,
            timeout=5,
        )
        return result.returncode == 0
    except Exception:
        return False
