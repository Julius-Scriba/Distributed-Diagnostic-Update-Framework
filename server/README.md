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
