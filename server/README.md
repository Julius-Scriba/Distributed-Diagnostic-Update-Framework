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
{ "cmd": "SAFE_MODE" }
```
Unterstützte Befehle sind `SAFE_MODE`, `WIPE` und `DEEP_SLEEP`. Diese werden serverseitig geloggt und beim nächsten Abruf ausgeliefert.

```
GET /commands/<uuid>
-> { "commands": ["SAFE_MODE"] }
```

```
pip install -r requirements.txt
python app.py
```
