# -*- mode: python ; coding: utf-8 -*-
# PyInstaller-Spec fuer "Eiswolfs iBoard.app" (Auftrag "installierbare
# App", 29.10.) - reine Menueleisten-App (LSUIElement), arm64, mit
# eingebettetem macmon-Binary. Aufruf: siehe CLAUDE.md "Installierbare
# App bauen".
import sys

block_cipher = None

APP_VERSION = "1.1"

a = Analysis(
    ["sender.py"],
    pathex=[],
    binaries=[("bundle_resources/macmon", ".")],
    datas=[("bundle_resources/macmon_LICENSE.txt", ".")],
    hiddenimports=["rumps", "psutil", "serial", "serial.tools.list_ports"],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    cipher=block_cipher,
)
pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name="Eiswolfs iBoard",
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=False,
    console=False,
    target_arch="arm64",
)

coll = COLLECT(
    exe,
    a.binaries,
    a.zipfiles,
    a.datas,
    strip=False,
    upx=False,
    name="Eiswolfs iBoard",
)

app = BUNDLE(
    coll,
    name="Eiswolfs iBoard.app",
    icon=None,
    bundle_identifier="com.alexdemurtas.eiswolfsiboard.app",
    info_plist={
        "CFBundleName": "Eiswolfs iBoard",
        "CFBundleDisplayName": "Eiswolfs iBoard",
        "CFBundleShortVersionString": APP_VERSION,
        "CFBundleVersion": APP_VERSION,
        "LSUIElement": True,  # reine Menueleisten-App, kein Dock-Symbol
        "NSHighResolutionCapable": True,
        "NSLocalNetworkUsageDescription": (
            "Eiswolfs iBoard sendet Systemwerte an dein Display im "
            "lokalen Netzwerk."
        ),
        "LSMinimumSystemVersion": "11.0",
        "LSArchitecturePriority": ["arm64"],
    },
)
