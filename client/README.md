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
