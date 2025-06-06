# Deployment Guide

This document outlines the recommended production setup for the ULTSPY C2 server and web interface.

## Directory Layout
```
/srv/ultspy-c2/
  backend      # Flask application
  frontend     # compiled React build
  venv         # Python virtual environment
```

## Build Steps
1. Build the web dashboard:
   ```bash
   cd frontend
   npm install
   npm run build
   cp -r dist /srv/ultspy-c2/frontend
   ```
2. Install and configure the backend:
   ```bash
   cd ../server
   python -m venv /srv/ultspy-c2/venv
   source /srv/ultspy-c2/venv/bin/activate
   pip install -r requirements.txt
   cp -r . /srv/ultspy-c2/backend
   ```
3. Start the API via Gunicorn:
   ```bash
   cd /srv/ultspy-c2/backend
   source ../venv/bin/activate
   gunicorn -w 4 -b 0.0.0.0:5000 wsgi:app
   ```
   Reverse proxy this socket under `/api/` using Nginx.

Logs will be written to STDOUT and should be collected via the chosen process manager.
