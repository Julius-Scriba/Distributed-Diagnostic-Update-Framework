# Client Agent

Der Agent lädt zur Laufzeit Module aus `plugins`. Neu ist ein Persistenzmechanismus für Windows und ein Safe‑Mode.

## Build
```
mkdir build && cd build
cmake ..
make
```

## Persistenz
Beim Start setzt das `Persistence`‑Plugin einen Run‑Key `SystemDiagnostics` sowie einen geplanten Task. Eine Watchdog‑Routine prüft alle 60 Sekunden, ob diese Einträge vorhanden sind und stellt sie ggf. wieder her.

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
