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
- Crypto: AES-256-CBC-Verschlüsselung der Datenübertragung

### Server (Python)

- REST-API (Flask)
- Agent-Registrierung & Heartbeats
- Kommandomodul pro Client
- Persistente Datenhaltung (SQLite, vorbereitbar für PostgreSQL)
- Loggingsystem
- Admin-Schnittstelle vorbereitet für spätere Erweiterungen

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
