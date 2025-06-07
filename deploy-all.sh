#!/bin/bash
set -euo pipefail

REPO_DIR="$HOME/Distributed-Diagnostic-Update-Framework"
BACKEND_DIR="$REPO_DIR/server"
FRONTEND_DIR="$REPO_DIR/frontend"
WEBROOT="/var/www/ultspy-dashboard"
BACKUP_DIR="${WEBROOT}-backups"
LOG_DIR="$REPO_DIR/deploy_logs"
VENVDIR="$BACKEND_DIR/venv"

mkdir -p "$BACKUP_DIR" "$LOG_DIR"
LOGFILE="$LOG_DIR/deploy-$(date +%Y%m%d-%H%M%S).log"
exec > >(tee -a "$LOGFILE") 2>&1

echo "[deploy] Starting deployment $(date)"

cd "$REPO_DIR"
echo "[deploy] Updating repository..."
if ! git pull --ff-only; then
  echo "[deploy] git pull failed" >&2
  exit 1
fi

# Backend setup
cd "$BACKEND_DIR"
if [ ! -d "$VENVDIR" ]; then
  echo "[deploy] Creating virtual environment"
  python3 -m venv "$VENVDIR"
fi
source "$VENVDIR/bin/activate"
echo "[deploy] Installing backend requirements"
pip install -r requirements.txt

echo "[deploy] Migrating database if needed"
python migrate_keys.py || true

systemctl restart ultspy.service || echo "[deploy] Warning: could not restart gunicorn service"

deactivate

# Frontend build
cd "$FRONTEND_DIR"
echo "[deploy] Installing frontend dependencies"
npm install --silent

echo "[deploy] Building frontend"
if ! npm run build --silent; then
  echo "[deploy] frontend build failed" >&2
  exit 1
fi

if [ -d "$WEBROOT" ]; then
  ts=$(date +%Y%m%d-%H%M%S)
  tar czf "$BACKUP_DIR/dashboard-$ts.tar.gz" -C "$WEBROOT" .
fi

rsync -a --delete "$FRONTEND_DIR/dist/" "$WEBROOT/"

# cleanup backups >10
ls -1t "$BACKUP_DIR" | tail -n +11 | xargs -r rm -f

echo "[deploy] Deployment finished $(date)"
