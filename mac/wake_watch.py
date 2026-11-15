"""Erkennung von Systemschlaf/Display-Schlaf (Auftrag "Ruhezustand-Fix"):
liefert is_awake(), das waehrend eines vollen Ruhezustands UND waehrend
eines DarkWake (Mac laeuft kurz mit Netzwerk, Display bleibt dunkel -
siehe pmset -g log) False liefert - genau das Fenster, in dem
sender.py.SenderThread KEINE Pakete mehr verschicken soll.

Zwei unabhaengige AppKit-Benachrichtigungspaare:
- NSWorkspaceWillSleepNotification/NSWorkspaceDidWakeNotification: echter
  System-Schlaf/-Aufwachen. DidWake feuert NICHT bei einem reinen
  DarkWake (das ist genau der in macOS dokumentierte Unterschied).
- NSWorkspaceScreensDidSleepNotification/...DidWakeNotification: der
  Bildschirm geht aus/an - bleibt waehrend eines DarkWake durchgehend
  "aus", da der Bildschirm dabei nicht angeht. Deckt damit genau die
  DarkWake-Luecke ab, die das erste Paar allein nicht sieht.

Sonderfall Mac ohne angeschlossenen Bildschirm (Auftrag: "KEINE
Sendepause durch die Display-Erkennung" in diesem Fall): has_display
wird einmalig beim Start ueber NSScreen.screens() geprueft - ist kein
Bildschirm da (oder PyObjC generell nicht verfuegbar), zaehlt nur noch
das System-Schlaf-Paar."""
import threading


def _noop_log(_msg):
    pass


_log_fn = _noop_log


def set_logger(fn):
    """Von sender.py gesetzt, um Ereignisse in die gemeinsame Log-Datei/
    Konsole zu schreiben, ohne einen Kreisimport zu brauchen."""
    global _log_fn
    _log_fn = fn or _noop_log


class WakeWatcher:
    def __init__(self, on_wake=None):
        self._on_wake = on_wake
        self._lock = threading.Lock()
        self._system_asleep = False
        self._screens_asleep = False
        self._has_display = True
        self._observer = None
        self._install()

    def is_awake(self):
        with self._lock:
            if self._system_asleep:
                return False
            if self._has_display and self._screens_asleep:
                return False
            return True

    def _apply(self, **changes):
        with self._lock:
            was_awake = not self._system_asleep and (not self._has_display or not self._screens_asleep)
            for key, value in changes.items():
                setattr(self, key, value)
            now_awake = not self._system_asleep and (not self._has_display or not self._screens_asleep)
        if now_awake and not was_awake and self._on_wake:
            try:
                self._on_wake()
            except Exception as exc:
                _log_fn(f"Fehler im Aufwach-Callback: {exc!r}")

    def _install(self):
        try:
            from AppKit import (
                NSScreen,
                NSWorkspace,
                NSWorkspaceWillSleepNotification,
                NSWorkspaceDidWakeNotification,
                NSWorkspaceScreensDidSleepNotification,
                NSWorkspaceScreensDidWakeNotification,
            )
            from Foundation import NSObject
        except Exception as exc:
            _log_fn(f"Schlaf-/Aufwach-Erkennung nicht verfuegbar: {exc!r} - "
                     "Sendepause bleibt aus, App sendet wie bisher durchgehend.")
            self._has_display = False
            return

        try:
            self._has_display = len(NSScreen.screens()) > 0
        except Exception:
            self._has_display = False
        if not self._has_display:
            _log_fn("Kein Bildschirm erkannt - Sendepause nur bei echtem Systemschlaf")

        watcher = self

        class _Observer(NSObject):
            def onWillSleep_(self, _n):
                watcher._apply(_system_asleep=True)
                _log_fn("Systemschlaf: eingeschlafen")

            def onDidWake_(self, _n):
                watcher._apply(_system_asleep=False)
                _log_fn("Systemschlaf: aufgewacht")

            def onScreensDidSleep_(self, _n):
                watcher._apply(_screens_asleep=True)
                _log_fn("Bildschirm: schlaeft (Display-Sleep/DarkWake)")

            def onScreensDidWake_(self, _n):
                watcher._apply(_screens_asleep=False)
                _log_fn("Bildschirm: wach")

        self._observer = _Observer.alloc().init()
        nc = NSWorkspace.sharedWorkspace().notificationCenter()
        nc.addObserver_selector_name_object_(
            self._observer, "onWillSleep:", NSWorkspaceWillSleepNotification, None
        )
        nc.addObserver_selector_name_object_(
            self._observer, "onDidWake:", NSWorkspaceDidWakeNotification, None
        )
        nc.addObserver_selector_name_object_(
            self._observer, "onScreensDidSleep:", NSWorkspaceScreensDidSleepNotification, None
        )
        nc.addObserver_selector_name_object_(
            self._observer, "onScreensDidWake:", NSWorkspaceScreensDidWakeNotification, None
        )
