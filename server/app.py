from flask import Flask, request, jsonify
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import logging

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///dduf.db'
db = SQLAlchemy(app)

logging.basicConfig(level=logging.INFO)

class Client(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    uuid = db.Column(db.String(64), unique=True, nullable=False)
    last_seen = db.Column(db.DateTime, default=datetime.utcnow)

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

@app.route('/commands/<int:client_id>', methods=['GET'])
def get_commands(client_id):
    return jsonify({'commands': []})  # placeholder

@app.route('/update/<int:client_id>', methods=['POST'])
def update(client_id):
    return jsonify({'status': 'received'})

if __name__ == '__main__':
    db.create_all()
    app.run(host='0.0.0.0', port=5000)
