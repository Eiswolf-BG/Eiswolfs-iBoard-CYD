#pragma once
// Schutz gegen Befehlswiederholung (Auftrag "Sicherheit und Lizenzen",
// Teil 2) - ersetzt den bisherigen Klartext-Token am Zeilenende durch
// einen steigenden Zaehler + eine HMAC-SHA256-Pruefsumme (Schluessel =
// Token). Der Token selbst geht dadurch NIE mehr im Klartext uebers
// Netz/USB. Siehe docs/TOKEN.md fuer die ehrliche Einordnung (das ist
// KEINE Verschluesselung - Befehle bleiben mitlesbar, nur nicht mehr
// faelsch- oder wiederholbar).
//
// Resync (verlorener Zaehler, z.B. nach Werksreset): CYD sendet dafuer
// einfach einen ganz normalen signierten "RESYNC"-Befehl - die
// eigentliche Sonderbehandlung (den neuen, nicht zwingend hoeheren
// Zaehlerwert trotzdem als neue Basis zu akzeptieren) passiert bewusst
// NUR empfaengerseitig, hinter einem manuellen, zeitlich begrenzten
// Schalter (siehe Mac-App/Windows-Listener) - das Geraet selbst kann
// nach einem Totalverlust seines eigenen Zaehlers unmoeglich beweisen,
// dass sein neuer Vorschlag wirklich "neuer" ist als vorher.
#include <Arduino.h>
#include "storage.h"

namespace CmdAuth {
    // Haengt " <Zaehler> <16-Hex-Zeichen-HMAC>" an `baseMessage` an und
    // erhoeht+speichert dabei den persistenten Zaehler (NVS/SD, siehe
    // Storage::DeviceConfig::cmdCounter) - JEDER Aufruf verbraucht einen
    // neuen Zaehlerwert, nie wiederverwendet. `cfg` per Referenz, da
    // `deviceCfg` in main.cpp in einem anonymen Namespace liegt (kein
    // "extern" ueber Uebersetzungseinheiten hinweg moeglich).
    String signCommand(const String& baseMessage, Storage::DeviceConfig& cfg);
}
