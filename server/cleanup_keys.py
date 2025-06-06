import datetime
from app import db, ApiKey

CUT_OFF_DAYS = 90

def main():
    now = datetime.datetime.utcnow()
    cutoff = now - datetime.timedelta(days=CUT_OFF_DAYS)
    stale = ApiKey.query.filter(
        (ApiKey.last_used_at == None) | (ApiKey.last_used_at < cutoff)
    ).filter(ApiKey.revoked == False).all()
    if not stale:
        print('no stale keys')
        return
    print('Stale API keys (>90 days unused):')
    for k in stale:
        last = k.last_used_at.isoformat() if k.last_used_at else 'never'
        print(f"{k.id} {k.name} last_used={last}")
    print('\nReview and revoke as needed.')

if __name__ == '__main__':
    with db.app.app_context():
        main()
