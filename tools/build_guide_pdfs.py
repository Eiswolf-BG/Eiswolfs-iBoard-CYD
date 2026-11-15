#!/usr/bin/env python3
"""Anleitungs-PDFs aus den Quelldateien docs/ANLEITUNG.de.md und docs/ANLEITUNG.en.md bauen.

Aufruf:  python3 tools/build_guide_pdfs.py <Ausgabeordner>
Braucht das Python-Paket "markdown" (z.B. in einem Wegwerf-venv: python3 -m venv /tmp/mdvenv &&
/tmp/mdvenv/bin/pip install markdown) und Google Chrome oder Microsoft Edge (Hintergrundmodus,
kein Zusatzprogramm). Die Markdown-Dateien sind die EINZIGE Quelle - nie die PDFs von Hand aendern.
Foto-Platzhalter ("[Foto: ...]" / "[Photo: ...]") erscheinen als graue Kaesten, bis echte Bilder
eingesetzt sind.
"""
import os
import re
import subprocess
import sys
import tempfile

import markdown

HERE = os.path.dirname(os.path.abspath(__file__))
DOCS = os.path.join(HERE, "..", "docs")
BROWSERS = [
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
    "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge",
]
CSS = """
@page { size: A4; margin: 18mm 16mm; }
body { font-family: -apple-system, 'Helvetica Neue', Arial, sans-serif; font-size: 10.5pt; line-height: 1.45; color: #111; }
h1 { font-size: 22pt; margin: 0 0 6pt; } h2 { font-size: 15pt; border-bottom: 1px solid #999; padding-bottom: 3pt; margin-top: 18pt; page-break-after: avoid; }
h3 { font-size: 12pt; page-break-after: avoid; }
code { background: #f1f1f1; padding: 1px 3px; border-radius: 3px; font-size: 9.5pt; }
pre { background: #f1f1f1; padding: 8px; border-radius: 4px; overflow-x: auto; page-break-inside: avoid; }
pre code { background: none; padding: 0; }
table { border-collapse: collapse; width: 100%; font-size: 9pt; } th, td { border: 1px solid #aaa; padding: 4px 6px; vertical-align: top; }
th { background: #e8e8e8; } tr { page-break-inside: avoid; }
blockquote { border-left: 4px solid #c90; background: #fff8e1; margin: 8pt 0; padding: 4pt 10pt; }
.photo { border: 2px dashed #999; background: #f5f5f5; color: #555; text-align: center; padding: 22pt 8pt; margin: 8pt 0; font-style: italic; page-break-inside: avoid; }
a { color: #0a58ca; text-decoration: none; }
"""


def find_browser():
    for b in BROWSERS:
        if os.path.exists(b):
            return b
    sys.exit("Weder Chrome noch Edge gefunden.")


def build(md_path, pdf_path, lang, browser):
    text = open(md_path, encoding="utf-8").read()
    # Interne Redaktionsnotiz (erster Zitatblock direkt unter der Ueberschrift) gehoert nicht ins PDF
    lines = text.split("\n")
    i = 1
    while i < len(lines) and lines[i].strip() == "":
        i += 1
    if i < len(lines) and lines[i].startswith(">"):
        j = i
        while j < len(lines) and lines[j].startswith(">"):
            j += 1
        text = "\n".join(lines[:i] + lines[j:])
    html_body = markdown.markdown(text, extensions=["tables", "fenced_code", "toc", "sane_lists"])
    html_body = re.sub(r"<p>\[((?:Foto|Photo): [^\]]*)\]</p>", r'<div class="photo">\1</div>', html_body)
    html_body = re.sub(r"\[((?:Foto|Photo): [^\]]*)\]", r'<div class="photo">\1</div>', html_body)
    html = '<!doctype html><html lang="%s"><head><meta charset="utf-8"><style>%s</style></head><body>%s</body></html>' % (
        lang, CSS, html_body)
    with tempfile.NamedTemporaryFile("w", suffix=".html", delete=False, encoding="utf-8") as f:
        f.write(html)
        tmp = f.name
    try:
        subprocess.run([browser, "--headless=new", "--disable-gpu", "--no-pdf-header-footer",
                        "--print-to-pdf=" + pdf_path, "file://" + tmp],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=120)
    finally:
        os.remove(tmp)


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        return 2
    out = os.path.abspath(sys.argv[1])
    os.makedirs(out, exist_ok=True)
    browser = find_browser()
    for lang, name in (("de", "Eiswolfs-iBoard-Anleitung-DE.pdf"), ("en", "Eiswolfs-iBoard-Guide-EN.pdf")):
        src = os.path.join(DOCS, "ANLEITUNG.%s.md" % lang)
        dst = os.path.join(out, name)
        build(src, dst, lang, browser)
        print("%s: %d Bytes" % (dst, os.path.getsize(dst)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
