"""Einmalige, zeitlich begrenzte lokale Freigabe der eigenen config.json
fuers Windows-Pendant (Auftrag "Token nicht abtippen muessen" - Cmd+C/
Strg+V geht ja nicht rechnerübergreifend). Startet einen winzigen HTTP-
Server im LAN, der GENAU EINMAL (oder nach TIMEOUT_S Sekunden) eine
fertige config.json fuer den Windows-Listener ausliefert, dann sich
selbst wieder abschaltet - der Token steht damit nur fuer ein kurzes
Fenster im Klartext im lokalen Netz, nicht dauerhaft.

Bewusst kein Verschluesselungs-/Auth-Aufwand: der Token selbst ist laut
docs/TOKEN.md kein hochsicheres Geheimnis ("muss nur schwer zu erraten
sein, nicht einem gezielten Angriff im eigenen LAN standhalten")."""
import http.server
import json
import socket
import threading

import config

PORT = 8765
TIMEOUT_S = 60


def _local_ip():
    """Ermittelt die eigene LAN-IP ueber einen (nie tatsaechlich
    sendenden) UDP-Socket - funktioniert auch ohne aktive Internet-
    verbindung, solange ein lokales Netz besteht."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        s.close()


class _Handler(http.server.BaseHTTPRequestHandler):
    server_version = "EiswolfsIBoard/1.0"

    def do_GET(self):
        token = config.get_fan_boost_token() or ""
        body = json.dumps({"token": token}, indent=2).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Disposition", 'attachment; filename="config.json"')
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)
        # kein explizites shutdown() noetig - handle_request() im
        # Aufrufer kehrt nach GENAU dieser einen Anfrage ohnehin zurueck.

    def log_message(self, format, *args):
        pass  # kein Logging - der Token soll nirgends landen


class _OneShotServer(http.server.HTTPServer):
    """Merkt sich, ob handle_request() wegen einer echten Anfrage oder
    wegen Zeitablauf zurueckkam (handle_timeout() wird nur im zweiten
    Fall aufgerufen) - socketserver bietet dafuer sonst kein Signal."""
    timed_out = False

    def handle_timeout(self):
        self.timed_out = True


def start(on_done=None):
    """Startet den Freigabe-Server in einem eigenen Thread, liefert die
    URL zurueck, die man auf dem Windows-Laptop im Browser oeffnet.
    on_done(served: bool) wird aufgerufen, wenn der Server endet (egal
    ob durch eine Anfrage oder durch den Timeout)."""
    server = _OneShotServer(("0.0.0.0", PORT), _Handler)
    server.timeout = TIMEOUT_S

    def _run():
        server.handle_request()  # blockiert bis 1 Anfrage ODER Timeout
        server.server_close()
        if on_done:
            on_done(not server.timed_out)

    threading.Thread(target=_run, daemon=True).start()
    return f"http://{_local_ip()}:{PORT}/"
