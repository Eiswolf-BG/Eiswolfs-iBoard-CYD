# Machbarkeitstest "Lüfter steuern" - Ergebnisse (Schritt B1)

## System
- macOS: 27.0.1 (Build 26A434)
- Modell: Mac mini, Model Identifier `Mac16,10`, Chip Apple M4
- Lüfter laut `macmon`: 1 Stück (`fan0`), `max_rpm` = 4900

## Lesbare Werte (ohne sudo, über `macmon`)

`macmon pipe -s 1` liefert für den Lüfter nur:
```json
{"fans": [{"name": "fan0", "rpm": <aktuelle Drehzahl>, "max_rpm": 4900}]}
```
Kein Minimum, kein Modus (Automatik/Manuell), keine Zieldrehzahl über
`macmon` auslesbar - nur Ist-Drehzahl und Maximalwert.

`ioreg -c AppleSMC -r` bestätigt den SMC-Treiber ist vorhanden und aktiv:
`AppleSMCClient` (IOUserClient), `CFBundleIdentifier` =
`com.apple.driver.AppleSMC`, `Generation` = 4. Kein zusätzliches
Kommandozeilen-Tool (`smc`, `istats`) war vorinstalliert.

## Schreibzugriff auf Apple Silicon (M4) - Recherche-Ergebnis

Mehrere unabhängige, aktuelle Open-Source-Projekte bestätigen: Ein
manueller Schreibzugriff auf die Lüfter-SMC-Schlüssel ist auf M3/M4
grundsätzlich MÖGLICH, aber durch die Firmware zusätzlich abgesichert:

- Ein offenes GitHub-Issue im Projekt "Stats"
  (https://github.com/exelban/stats/issues/2928) beschreibt, dass
  direkte Schreibversuche auf die Lüfter-Modus-/Zieldrehzahl-Schlüssel
  auf M3/M4 vom System-Thermal-Dienst (`thermalmonitord`) ignoriert
  bzw. sofort zurückgesetzt werden, SOLANGE nicht vorher ein
  zusätzlicher "Force-Test"-Freigabeschlüssel (in der Community häufig
  als `Ftst`-artiger Schlüssel bezeichnet) gesetzt wurde.
- Das Projekt "ThermalForge"
  (https://github.com/ProducerGuy/ThermalForge) und "MacFanControl"
  (https://github.com/raminsharifi/MacFanControl) geben beide
  ausdrücklich M1-M4-Unterstützung an und behaupten, diesen
  Freigabe-Mechanismus bereits zu beherrschen.
- Ein Linux-Kernel-Patch für den `macsmc`-Treiber
  (https://github.com/aurora-silicon/linux/pull/512, Oktober 2026)
  beschreibt denselben Mechanismus im Detail für ein M4-Pro-Gerät:
  beide Lüfter starten in einem firmwareeigenen "Systemmodus 3", ein
  direkter Wechsel in den manuellen Modus wird abgelehnt; der
  funktionierende Ablauf ist: Freigabeschlüssel setzen -> warten, bis
  der Lüfter den Systemmodus verlässt -> erst dann die Zieldrehzahl
  schreiben -> den Freigabeschlüssel beim Zurückschalten auf Automatik
  wieder löschen.

**Wichtiger Vorbehalt**: Keine dieser Quellen ist offizielle
Apple-Dokumentation. Schlüsselnamen und Timing-Werte stammen aus
Reverse-Engineering einzelner Communities und können je nach genauem
Modell/Firmware-/macOS-Stand abweichen. Es gibt keine Garantie, dass
der auf einem M4 Pro beobachtete Mechanismus 1:1 auf diesen Mac mini
M4 (`Mac16,10`) zutrifft.

## Entscheidung an dieser Stelle

Einen SMC-Schreibzugriff von Grund auf selbst nachzubauen (rohe
IOKit-`IOConnectCallStructMethod`-Aufrufe mit den exakten, nicht
dokumentierten Struct-Layouts) ist mit echtem Risiko verbunden, falls
ein Byte-Offset oder Freigabe-Timing nicht stimmt - im schlimmsten Fall
ein hängender Kernel-Dienst oder ein Systemabsturz, bei einem laufenden
Produktiv-Mac ohne Backup-Sicherheitsnetz für diesen einen Testlauf.
Der Auftrag erlaubt ausdrücklich, stattdessen ein bekanntes Open-Source-
Tool zum Vergleich zu nennen und VOR der Installation nachzufragen -
siehe Rückfrage im Chat.
