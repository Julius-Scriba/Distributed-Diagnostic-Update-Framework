# Client Agent

Der Agent lädt zur Laufzeit Module aus `plugins`. Neu ist ein Persistenzmechanismus für Windows und ein Safe‑Mode.
Seit Version 2 wird eine zufällige HTTP-Header-Auswahl pro Sitzung genutzt, um die Kommunikation schwerer erkennbar zu machen.
Ab Version 4 werden alle HTTP-Aufrufe mit einer HMAC-SHA256 Signatur versehen. Ein Nonce und Timestamp verhindern Replay-Attacken.
Ab Version 3 liest ein neues `ConfigModule` Einstellungen aus `agent_config.json`, sodass Ziel-URLs und Host-Header flexibel gesetzt werden können.

## Build
```
mkdir build && cd build
cmake ..
make
```

## Konfiguration
Eine Datei `agent_config.json` im gleichen Verzeichnis erlaubt die Vorgabe von
`server_url`, optionalem `host_header` und einem `path_prefix` für alle HTTP-Aufrufe.
Beispiel:
```json
{
  "server_url": "http://my.cdn.tld",
  "host_header": "origin.example.com",
  "path_prefix": "/api"
}
```

## Persistenz
Der neue Persistenz-Layer nutzt mehrere Mechanismen:
- Ein Run‑Key mit PowerShell-EncodedCommand startet den Agenten nach dem Login.
- Ein versteckter Scheduled Task mit Verzögerung sorgt für Redundanz.
- Die Binärdatei wird zusätzlich verschlüsselt in einem NTFS-ADS hinterlegt.
- Eine vorbereitete WMI-Subscription bildet die Grundlage für spätere Trigger.

## Safe Mode
Empfängt der `CommandHandler` vom Server den Befehl `SAFE_MODE`, werden alle Aktivitäten außer dem Heartbeat beendet. Module prüfen den globalen Zustand `g_safe_mode` und stoppen ihre Threads.

## Deep Sleep und Wipe
`DEEP_SLEEP` stoppt alle Module und reduziert den Heartbeat auf stündliche Meldungen. Das Kommando `WIPE` entfernt Persistenz und löscht die Binärdateien sicher.

## IPC-Basis
Ein neues IPC-Framework stellt eine Named-Pipe `\\.\pipe\US_IPC_CORE` bereit. Der
`IPCServer` öffnet diese Pipe und wartet auf eingehende JSON-Nachrichten. Module
können über `IPCClient::send(json)` Daten an den Server schicken. Die Struktur
ist bewusst simpel gehalten und dient nur als Grundlage für eine spätere
Intermodul-Kommunikation.

## Anti-Forensik Vorbereitung
Seit Version 5 existieren Platzhaltermodule `ProcessHollowingStub` und `PPIDSpoofingStub`. Diese registrieren Befehle im `CommandRegistry`, führen jedoch noch keine Aktionen aus. Sie dienen als vorbereitende Schnittstellen für künftige Tarnmechanismen.

## Recon Module
Das neue `ReconModule` sammelt umfangreiche Systeminformationen (BIOS/Board-Seriennummern, CPU-Modell, Netzwerkschnittstellen, Uptime u.v.m.) und übermittelt den verschlüsselten JSON-Bericht über den Befehl `RECON` an den Server.
