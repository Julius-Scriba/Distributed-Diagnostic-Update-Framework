import json
import uuid
import bcrypt
from app import db, ApiKey, CONFIG


def main():
    keys = CONFIG.get('api_keys', {})
    if not keys:
        print('no keys in config.json')
        return
    created = False
    for name, key in keys.items():
        if not ApiKey.query.filter_by(name=name).first():
            hashed = bcrypt.hashpw(key.encode(), bcrypt.gensalt()).decode()
            db.session.add(ApiKey(id=str(uuid.uuid4()), name=name, hashed_key=hashed))
            created = True
    if created:
        db.session.commit()
        cfg_path = 'config.json'
        try:
            with open(cfg_path, 'r+') as f:
                cfg = json.load(f)
                cfg.pop('api_keys', None)
                f.seek(0)
                json.dump(cfg, f, indent=2)
                f.truncate()
            print('migrated keys to database')
        except Exception as e:
            print('migrated but failed to update config:', e)
    else:
        print('no new keys')

if __name__ == '__main__':
    with db.app.app_context():
        db.create_all()
        main()
