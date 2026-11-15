# Auftrag "Sicherheit und Lizenzen" (Teil 1b-3): der Compiler bettet
# ueber __FILE__/Assert-Makros die ABSOLUTEN Quellpfade der Arduino-
# Core-Bibliotheken (unter ~/.platformio/...) im Klartext in die
# fertige firmware.bin ein - das verraet den lokalen Benutzernamen in
# jeder veroeffentlichten Firmware-Datei. -ffile-prefix-map ersetzt
# diese Pfade beim Kompilieren durch neutrale Platzhalter.
#
# Bewusst als PRE-SCRIPT statt fest in platformio.ini eingetragen: der
# echte lokale Pfad wird hier zur BAUZEIT aus der Umgebung ermittelt
# (os.path.expanduser), steht also nirgends als Text im Repo - jeder,
# der dieses Projekt auf einem anderen Rechner/unter einem anderen
# Benutzernamen baut, bekommt automatisch SEINEN eigenen Pfad
# rausgefiltert, ohne diese Datei anpassen zu muessen.
Import("env")
import os

home = os.path.expanduser("~")
pio_core = env.get("PROJECT_CORE_DIR") or os.path.join(home, ".platformio")
project_dir = env["PROJECT_DIR"]

env.Append(
    CCFLAGS=[
        "-ffile-prefix-map=" + pio_core + "=/pio",
        "-ffile-prefix-map=" + project_dir + "=/project",
        # Fallback fuers Heimverzeichnis selbst, falls ein Pfad ausserhalb
        # von project_dir/pio_core liegt (z.B. $TMPDIR bei manchen Setups).
        "-ffile-prefix-map=" + home + "=/home",
    ]
)
