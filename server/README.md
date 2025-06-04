# Server Component

Eine minimale Flask-Anwendung, die REST-Endpunkte für die Registrierung von Clients sowie für Befehle und Updates bereitstellt.

Der Endpunkt `/key_exchange` übernimmt den sicheren Schlüsselaustausch:

```text
POST /key_exchange
{
  "uuid": "<client uuid>",
  "public_key": "-----BEGIN RSA PUBLIC KEY-----..."
}
```
Antwort:

```json
{"aes_key": "<base64 encrypted key>"}
```

```
pip install -r requirements.txt
python app.py
```
