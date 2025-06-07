# Deployment Guide

This document outlines the recommended production setup for the ULTSPY C2 server and web interface.

## Directory Layout
The deployment places the Flask backend under the repository directory while the
compiled frontend is served from `/var/www/ultspy-dashboard/`.

```
/home/<user>/Distributed-Diagnostic-Update-Framework/
  server/     # Flask application
  client/     # C++ agent sources
  frontend/   # React source (dist gets synced)
  deploy-all.sh
/var/www/ultspy-dashboard/  # live dashboard
```

## Automated Deployment
Run `./deploy-all.sh` from the repository root. The script will:

1. Pull the latest git changes
2. Update the Python virtual environment in `server/venv` and install
   requirements
3. Compile the client in `client/dev_version` and `client/deployable_version`
   using CMake
4. Restart the `ultspy.service` Gunicorn unit
5. Build the frontend and synchronise `frontend/dist/` to
   `/var/www/ultspy-dashboard/`

Existing dashboard files are archived in `/var/www/ultspy-dashboard-backups` and
only the ten newest backups are retained.
