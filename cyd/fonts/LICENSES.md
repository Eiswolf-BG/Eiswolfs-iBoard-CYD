# Schriften in Eiswolfs iBoard

Eiswolfs iBoard bietet zwei umschaltbare Anzeige-Schriften (siehe Menü/Antippen
der Kopfzeile am Gerät, Feld `font` in `config.json`). Beide sind frei
nutzbare Schriften:

## Schrift A "Pixel" - Press Start 2P

- Quelle: Google Fonts (`google/fonts`, Ordner `ofl/pressstart2p`),
  Copyright 2012 The Press Start 2P Project Authors (cody@zone38.net).
- Lizenz: SIL Open Font License, Version 1.1 (OFL) - voller Text in
  `cyd/src/fonts/PressStart2P_OFL.txt`.
- In diesem Projekt selbst (per `ttf_to_gfxfont.py`, nicht Teil des
  Firmware-Builds, nur zur einmaligen Konvertierung benutzt) aus der
  TTF-Datei in das von TFT_eSPI (GFXFF) erwartete C-Array-Format
  umgewandelt: `cyd/src/fonts/PixelFont20.h` (Basisgröße 20px, wird im
  Code per `tft.setTextSize()` hoch- oder runterskaliert).

## Schrift B "Klar" - FreeSansBold

- Herkunft: Adafruit-GFX-Library (BSD-Lizenz, Copyright Adafruit
  Industries), bereits als Teil der TFT_eSPI-Bibliothek unter
  `Fonts/GFXFF/FreeSansBold*.h` mitgeliefert (lib_deps, wird bei jedem
  Build automatisch von PlatformIO geholt - deshalb keine eigene Kopie
  im Projekt nötig). Voller Lizenztext in `freesansbold_bsd_license.txt`
  in diesem Ordner (Kopie des `license.txt` aus der TFT_eSPI-Bibliothek).
- Verwendete Größen: `FreeSansBold9pt7b`/`FreeSansBold18pt7b`/
  `FreeSansBold24pt7b`, je nach verfügbarem Platz in der Kachel
  automatisch gewählt (siehe `main.cpp::pickFont()`).
