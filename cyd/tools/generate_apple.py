#!/usr/bin/env python3
"""Erzeugt das Bootscreen-Apfel-Bild fuer Eiswolfs iBoard.

Auftrag "Bootscreen im Dashboard-Stil" (02.11., Korrektur nach Alex'
Rueckmeldung): Apfel NUR als KONTUR (Umriss) in GENAU EINER Gruenfarbe
(derselben Dashboard-Konstante wie der FAN-Streifen, TFT_GREEN =
0x07E0 = RGB(0,255,0)) - die Innenflaeche bleibt schwarz/Hintergrund.
KEIN Farbverlauf, KEIN Glanzfleck, KEIN Dithering mehr (loeste vorher
den "mintgruen/blaugruen, verwaschen" wirkenden, flaechig gefuellten
Apfel ab). Stiel und Blatt in derselben Gruenfarbe (kleine, gefuellte
Formen - eine Kontur waere dort zu duenn, um erkennbar zu sein), durch
einen schmalen schwarzen "Hof" von der Apfel-Kontur optisch getrennt.

Eigener Entwurf (kein Logo-Nachbau): runder Apfel ohne Biss, zwei
weiche Wuelste oben mit kleiner Einbuchtung, unten leicht schmaler mit
kleiner Einbuchtung. Kontur per dicht abgetastetem Catmull-Rom-Spline
(viele Stuetzpunkte -> wirkt wie eine echte Bezier-Kurve) in 4-facher
Aufloesung GRUEN AUF SCHWARZ gezeichnet, dann mit LANCZOS verkleinert
(weiche Kanten, keine Pixeloptik) - Mischfarben entstehen dadurch nur
an den Kanten zwischen Schwarz und Gruen, nirgends sonst. Die
Farbumwandlung nach RGB565 rundet jeden Pixel EXAKT (kein Floyd-
Steinberg-Fehlerstreuen mehr noetig, da es keinen Verlauf mehr gibt,
der in Stufen zerfallen koennte) - reines Gruen/Schwarz rundet dadurch
exakt auf 0x07E0/0x0000.

Ausgabe: apple_preview(.png/_3x.png) (Vorschau, IMMER vor dem Flashen
pruefen) + apple_rgb565.h (C-Array fuer TFT_eSPI, PROGMEM - die
Firmware zeichnet es mit tft.setSwapBytes(true) vor pushImage(), siehe
main.cpp).

Aufruf: python3 generate_apple.py
"""
import os

from PIL import Image, ImageChops, ImageDraw, ImageFilter

SCALE = 4
FINAL_W, FINAL_H = 116, 125
W, H = FINAL_W * SCALE, FINAL_H * SCALE

BG = (0x00, 0x00, 0x00)
# TFT_GREEN (0x07E0) als 8-Bit-RGB - EXAKT dieselbe Dashboard-Konstante
# wie der FAN-Streifen (siehe main.cpp TILES[]), keine neue Farbe.
APPLE_GREEN = (0x00, 0xFF, 0x00)
STEM_WIDTH_FINAL_PX = 6  # Zielgroesse (nach dem Downscale)
GAP_FINAL_PX = 2  # Breite des schwarzen Hofs um Stiel/Blatt (nach dem Downscale)

OUT_DIR = os.path.dirname(os.path.abspath(__file__))


def catmull_rom(points, samples_per_seg=40):
    """Dicht abgetasteter Catmull-Rom-Spline durch die Kontrollpunkte -
    bei genuegend Abtastpunkten optisch nicht von einer echten Bezier-
    Kurve zu unterscheiden, ganz ohne externe Bibliothek."""
    pts = [points[0]] + list(points) + [points[-1]]
    out = []
    for i in range(1, len(pts) - 2):
        p0, p1, p2, p3 = pts[i - 1], pts[i], pts[i + 1], pts[i + 2]
        for s in range(samples_per_seg):
            t = s / samples_per_seg
            t2 = t * t
            t3 = t2 * t
            x = 0.5 * ((2 * p1[0]) + (-p0[0] + p2[0]) * t +
                       (2 * p0[0] - 5 * p1[0] + 4 * p2[0] - p3[0]) * t2 +
                       (-p0[0] + 3 * p1[0] - 3 * p2[0] + p3[0]) * t3)
            y = 0.5 * ((2 * p1[1]) + (-p0[1] + p2[1]) * t +
                       (2 * p0[1] - 5 * p1[1] + 4 * p2[1] - p3[1]) * t2 +
                       (-p0[1] + 3 * p1[1] - 3 * p2[1] + p3[1]) * t3)
            out.append((x, y))
    out.append(points[-1])
    return out


OUTLINE_FINAL_PX = 3  # Zieldicke der Apfel-Kontur in End-Aufloesung


def build_body_outline_mask():
    """Apfelkoerper NUR als Kontur (Auftrag-Korrektur: Alex wollte einen
    reinen Umriss statt einer flaechig gefuellten Form) - gleiche
    spline-interpolierte Silhouette wie zuvor (Position/Groesse/Form
    unveraendert), aber nur ein schmaler Ring entlang des Randes bleibt
    gefuellt (Innenflaeche bleibt schwarz/Hintergrund). Erosion per
    MinFilter auf der SCHARFEN (noch ungeblurrten) Maske, danach erst
    das Ergebnis leicht weichzeichnen - so bleibt die Ringbreite
    gleichmaessig, statt durch eine vorher schon weiche Kante zu
    verschwimmen."""
    cx = W / 2.0
    top = 0.03 * H
    bottom = 0.99 * H
    body_h = bottom - top

    control = [
        (0.00 * 0.46 * W, top + 0.02 * body_h),
        (0.40 * 0.46 * W, top + 0.05 * body_h),
        (0.80 * 0.46 * W, top + 0.14 * body_h),
        (0.97 * 0.46 * W, top + 0.27 * body_h),
        (1.00 * 0.46 * W, top + 0.45 * body_h),
        (0.95 * 0.46 * W, top + 0.65 * body_h),
        (0.80 * 0.46 * W, top + 0.84 * body_h),
        (0.55 * 0.46 * W, top + 0.95 * body_h),
        (0.18 * 0.46 * W, top + 1.00 * body_h),
    ]

    right = catmull_rom(control, samples_per_seg=40)
    left = [(-x, y) for (x, y) in reversed(right)]
    outline_pts = [(cx + x, y) for (x, y) in right] + [(cx + x, y) for (x, y) in left]

    sharp = Image.new("L", (W, H), 0)
    ImageDraw.Draw(sharp).polygon(outline_pts, fill=255)

    d = ImageDraw.Draw(sharp)
    notch_w = int(0.055 * W)
    notch_h = int(0.09 * H)
    notch_y = int(top + 0.025 * body_h)
    d.ellipse([cx - notch_w, notch_y - notch_h, cx + notch_w, notch_y + notch_h], fill=0)

    bnotch_w = int(0.045 * W)
    bnotch_h = int(0.03 * H)
    d.ellipse([cx - bnotch_w, bottom - bnotch_h, cx + bnotch_w, bottom + bnotch_h], fill=0)

    erode_r = OUTLINE_FINAL_PX * SCALE
    ksize = erode_r * 2 + 1  # MinFilter braucht eine ungerade Kerngroesse
    eroded = sharp.filter(ImageFilter.MinFilter(ksize))
    ring = ImageChops.subtract(sharp, eroded)
    # Leichtes Weichzeichnen NUR fuer eine glatte Kantenoptik nach dem
    # Downscale, nicht zur Verbreiterung des Rings.
    ring = ring.filter(ImageFilter.GaussianBlur(radius=SCALE * 0.5))
    return ring


def draw_stem(canvas, mask):
    """Nur der Stiel (Auftrag-Korrektur: Alex wollte kein Blatt mehr) -
    in DERSELBEN Gruenfarbe wie der Koerper, durch einen schmalen
    schwarzen "Hof" von der Apfel-Kontur optisch getrennt (sonst waere
    er bei identischer Farbe dort, wo er die Kontur beruehrt, nicht von
    ihr zu unterscheiden)."""
    d = ImageDraw.Draw(canvas)
    cx = W // 2
    body_top = int(0.03 * H)
    stem_w = max(2, STEM_WIDTH_FINAL_PX * SCALE // 2)
    gap = GAP_FINAL_PX * SCALE
    halo_w = stem_w + 2 * gap

    stem_top = (cx - int(0.01 * W), body_top - int(0.06 * H))
    stem_bottom = (cx + int(0.02 * W), body_top + int(0.01 * H))

    # 1) Schwarzer Hof zuerst.
    d.line([stem_top, stem_bottom], fill=BG, width=halo_w)
    rh = halo_w // 2
    d.ellipse([stem_top[0] - rh, stem_top[1] - rh, stem_top[0] + rh, stem_top[1] + rh], fill=BG)
    d.ellipse([stem_bottom[0] - rh, stem_bottom[1] - rh, stem_bottom[0] + rh, stem_bottom[1] + rh], fill=BG)

    # 2) Eigentlicher gruener Stiel, kleiner, obendrauf.
    d.line([stem_top, stem_bottom], fill=APPLE_GREEN, width=stem_w)
    r = stem_w // 2
    d.ellipse([stem_top[0] - r, stem_top[1] - r, stem_top[0] + r, stem_top[1] + r], fill=APPLE_GREEN)
    d.ellipse([stem_bottom[0] - r, stem_bottom[1] - r, stem_bottom[0] + r, stem_bottom[1] + r], fill=APPLE_GREEN)


def to_rgb565_exact(img: Image.Image):
    """Rundet jeden Pixel EXAKT auf RGB565 (kein Fehlerstreuen/Dithering
    mehr - ohne Farbverlauf gibt es nichts mehr, das in Stufen zerfallen
    koennte). Reines Gruen (0,255,0) und reines Schwarz (0,0,0) runden
    dabei exakt auf 0x07E0 bzw. 0x0000 - nur echte Kantenpixel
    (Antialiasing zwischen Gruen und Schwarz) ergeben Mischfarben."""
    w, h = img.size
    px = img.load()
    rgb565 = [[0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            r, g, b = px[x, y][:3]
            r5 = (r * 31 + 127) // 255
            g6 = (g * 63 + 127) // 255
            b5 = (b * 31 + 127) // 255
            rgb565[y][x] = (r5 << 11) | (g6 << 5) | b5
    return rgb565


def main():
    outline_mask = build_body_outline_mask()

    canvas = Image.new("RGB", (W, H), BG)
    flat_green = Image.new("RGB", (W, H), APPLE_GREEN)
    canvas.paste(flat_green, (0, 0), outline_mask)

    draw_stem(canvas, outline_mask)

    final = canvas.resize((FINAL_W, FINAL_H), resample=Image.LANCZOS)

    preview_path = os.path.join(OUT_DIR, "apple_preview.png")
    final.save(preview_path)

    preview_big = final.resize((FINAL_W * 3, FINAL_H * 3), resample=Image.NEAREST)
    preview_big.save(os.path.join(OUT_DIR, "apple_preview_3x.png"))

    rgb565_rows = to_rgb565_exact(final)
    rgb565 = [v for row in rgb565_rows for v in row]

    # Pruefung (Auftrag-Korrektur: jetzt eine reine Kontur, die Mitte ist
    # also ABSICHTLICH schwarz) - die Bildmitte muss 0x0000 sein, UND
    # mindestens ein Pixel im Bild muss exakt TFT_GREEN (0x07E0) sein
    # (die Kontur selbst).
    center_val = rgb565_rows[FINAL_H // 2][FINAL_W // 2]
    assert center_val == 0x0000, f"Mitte des Apfels ist nicht schwarz (0x0000), sondern 0x{center_val:04X}"
    assert any(v == 0x07E0 for row in rgb565_rows for v in row), "Kein Pixel entspricht exakt TFT_GREEN (0x07E0)"

    header_path = os.path.normpath(os.path.join(OUT_DIR, "..", "src", "apple_rgb565.h"))
    with open(header_path, "w") as f:
        f.write("// Automatisch generiert von tools/generate_apple.py - nicht von Hand\n")
        f.write("// bearbeiten. Eigener Apfel-Entwurf, kein Logo-Nachbau. Flaches Gruen\n")
        f.write(f"// (TFT_GREEN/0x07E0, Dashboard-Palette), {FINAL_W}x{FINAL_H} Pixel, kein\n")
        f.write("// Verlauf/Glanz/Dithering mehr - direkt auf Hintergrund #000000\n")
        f.write("// gerendert. Firmware zeichnet mit tft.setSwapBytes(true) vor\n")
        f.write("// pushImage(), siehe main.cpp.\n")
        f.write("#pragma once\n#include <Arduino.h>\n\n")
        f.write(f"constexpr int16_t APPLE_W = {FINAL_W};\n")
        f.write(f"constexpr int16_t APPLE_H = {FINAL_H};\n")
        f.write("const uint16_t APPLE_RGB565[] PROGMEM = {\n")
        for i, v in enumerate(rgb565):
            f.write(f"0x{v:04X},")
            if (i + 1) % 16 == 0:
                f.write("\n")
        f.write("\n};\n")

    size_bytes = len(rgb565) * 2
    print(f"OK: {FINAL_W}x{FINAL_H} = {len(rgb565)} Pixel, {size_bytes} Bytes ({size_bytes/1024:.1f} KB)")
    print(f"Mitte des Apfels: 0x{center_val:04X} (erwartet 0x0000 = schwarz, reine Kontur)")
    print(f"Vorschau: {preview_path}")
    print(f"Header: {header_path}")


if __name__ == "__main__":
    main()
