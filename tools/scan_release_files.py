#!/usr/bin/env python3
"""Release-Dateien auf persoenliche Daten durchsuchen (auch Binaerdateien).

Aufruf:  python3 tools/scan_release_files.py <Datei-oder-Ordner> [...]
         (fuer die Mac-App mit dem Python aus mac/venv, damit auch der
          komprimierte Python-Code im .app-Buendel geprueft wird)

WICHTIG: Die Suchbegriffe stehen NIRGENDS in Dateien, sie werden zur Laufzeit
abgeleitet: Mac-/Windows-Benutzername aus dem Home-Verzeichnis, echter Name aus
dem Systemkonto (nur macOS/Linux), dazu optional Zusatzbegriffe ueber die
Umgebungsvariable SCAN_EXTRA (kommagetrennt, z.B. eine private E-Mail-Adresse,
die der Aufrufer selbst einsetzt). Ausgegeben wird NUR die Trefferzahl je Datei,
nie ein Suchbegriff oder ein Fundstueck. Ziel: ueberall 0 Treffer (Exit-Code 0).
"""
import os
import sys
import zlib
import zipfile
import marshal


def patterns():
    pats = set()
    home = os.path.basename(os.path.expanduser("~"))
    if len(home) >= 3:
        pats.add(home)
        pats.add("/Users/" + home)
    try:
        import pwd
        full = pwd.getpwuid(os.getuid()).pw_gecos.split(",")[0].strip()
        if len(full) >= 4:
            pats.add(full)
            pats.add(full.replace(" ", ""))
            last = full.split()[-1] if " " in full else ""
            if len(last) >= 4:
                pats.add(last)
    except Exception:
        pass
    for p in os.environ.get("SCAN_EXTRA", "").split(","):
        p = p.strip()
        if len(p) >= 3:
            pats.add(p)
    return sorted(pats)


def count_hits(data, pats):
    low = data.lower()
    return sum(low.count(p.lower().encode("utf-8")) for p in pats)


def scan_file(path, pats):
    try:
        with open(path, "rb") as f:
            data = f.read()
    except OSError:
        return 0
    hits = count_hits(data, pats)
    # Komprimierte Inhalte mit pruefen: ZIP-Mitglieder und rohe zlib-Strom-Stuecke
    if zipfile.is_zipfile(path):
        try:
            with zipfile.ZipFile(path) as z:
                for n in z.namelist():
                    hits += count_hits(n.encode("utf-8"), pats)
                    try:
                        hits += count_hits(z.read(n), pats)
                    except Exception:
                        pass
        except Exception:
            pass
    return hits


def scan_pyz(exe, pats):
    """PyInstaller-Buendel: Code-Objekte im PYZ auf Pfade pruefen (nur mit PyInstaller im Python)."""
    try:
        from PyInstaller.archive.readers import CArchiveReader, ZlibArchiveReader
    except Exception:
        return None
    hits = 0
    try:
        ca = CArchiveReader(exe)
        for name in ca.toc:
            if not name.endswith(".pyz"):
                continue
            tmp = os.path.join(os.environ.get("TMPDIR", "/tmp"), "scan_chk.pyz")
            with open(tmp, "wb") as f:
                f.write(ca.extract(name))
            z = ZlibArchiveReader(tmp)
            def walk(code):
                nonlocal hits
                hits += count_hits(code.co_filename.encode("utf-8"), pats)
                for c in code.co_consts:
                    if hasattr(c, "co_filename"):
                        walk(c)
            for mod in z.toc:
                try:
                    code = z.extract(mod)
                except Exception:
                    continue
                if code is not None:
                    walk(code)
            os.remove(tmp)
    except Exception:
        return None
    return hits


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    pats = patterns()
    total = 0
    files = 0
    for arg in sys.argv[1:]:
        paths = []
        if os.path.isdir(arg):
            for root, _, names in os.walk(arg):
                for n in names:
                    paths.append(os.path.join(root, n))
        else:
            paths.append(arg)
        for p in paths:
            if os.path.islink(p) or not os.path.isfile(p):
                continue
            h = scan_file(p, pats)
            files += 1
            total += h
            if h:
                print("TREFFER %5d  %s" % (h, p))
        if arg.rstrip("/").endswith(".app"):
            exe_dir = os.path.join(arg, "Contents", "MacOS")
            if os.path.isdir(exe_dir):
                for n in os.listdir(exe_dir):
                    r = scan_pyz(os.path.join(exe_dir, n), pats)
                    if r is None:
                        print("Hinweis: PYZ-Pruefung nicht moeglich (mit dem Python aus mac/venv starten)")
                    else:
                        total += r
                        print("PYZ-Code-Pfade in %s: %d Treffer" % (n, r))
    print("%d Suchbegriffe, %d Dateien geprueft, Treffer gesamt: %d" % (len(pats), files, total))
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
