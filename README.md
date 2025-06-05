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

Administratoren können über die API-Key geschützten Endpunkte `/admin/agents`,
`/admin/logs/<uuid>` und `/admin/command/<uuid>` auf den Server zugreifen. Der
API-Schlüssel wird via `X-API-KEY` Header übermittelt.

### Web UI

 A React-based interface in `frontend/` allows operators to log in with their API key and manage agents. The **Agents** page retrieves data from `/admin/agents` and shows the online state of each client.
 Start the development server with:

```bash
cd frontend
npm install
npm run dev
```
