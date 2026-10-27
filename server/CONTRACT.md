# Persistence API v1 — commit 1 of 8

## Delivered
Flask app factory, explicit SQLite initialization, schema v1, static frontend
serving and GET /api/v1/health. No session/checkpoint/score HTTP operations yet.
No game code, Web build or running port 8000 server is changed.

Health opens storage read-only. Recognized application/version/tables/columns
and checkpoint foreign key return 200 with status=ok and schema_version=1.
Missing, incompatible or unreadable storage returns 503 with
{"error":{"code":"storage_unavailable"}}. This is a readiness check, not a full
integrity scan. Health never initializes storage or reveals database paths.
API responses use no-store. Unknown API paths are not served as frontend files.
Static path resolution rejects traversal and symlinks escaping WEB_ROOT.

## Storage
Default DATABASE is server/instance/game.sqlite3, ignored by Git and outside
web/build. SPACE_DATABASE and SPACE_WEB_ROOT can override absolute paths;
DATABASE inside the public tree is refused. Production should use a private
persistent directory outside the checkout. Never commit databases, secrets,
journals or backups; custom paths need appropriate ignore/deployment rules.
Explicit init-db is transactional and idempotent. Foreign/nonempty or unknown
versions are refused, not reset. Each connection enables foreign keys and closes.
New database files use 0600 on POSIX; existing permissions remain operator-owned.
WAL is not enabled by this code. No automatic schema migration is performed.

## Planned data contract, not yet exposed
Session token hashes own checkpoints. Phase 2 must add secure anonymous cookies,
CSRF/origin checks, bounded JSON and request validation before exposing writes.
No client-supplied owner, SQL or arbitrary file path is accepted. Anonymous
cookie loss means losing access; accounts/recovery are not implemented here.

Checkpoint format_version=1: zero-based level_index 0..3, lives 1..9 and integer
score 0..2147473647. The score bound leaves margin for checked C reward arithmetic.
JSON booleans must be rejected as integers by the future API. PUT carries
expected_revision (0 for creation); server-owned revision starts at 1. Stale
writes must return 409, not overwrite a newer save. Dates/revision are server-owned.
Save between levels, not mid-frame. LOAD must not call new_game and erase stats;
reward-threshold restoration must be explicitly reviewed/tested at C integration.
Level index 3 may be stored although its configuration is unavailable: preserve
the save and report unavailable, never early victory. Game Over must not erase
an existing checkpoint. Ranking remains unverified, not an anti-cheat guarantee.
Future images/sounds use controlled asset IDs; no asset catalog is built now.

## Validation from repository root
python3 -m venv server/.venv
server/.venv/bin/python -m pip install -r server/requirements.txt
server/.venv/bin/python -B -m unittest server.test_db server.test_server -v

Tests use temporary files and Flask test_client, not a listening server.
For intentional initialization only:
server/.venv/bin/python -m flask --app server.app:create_app init-db

Do not replace/start the current port 8000 server during these tests. A later
manual smoke test may intentionally replace the static development server;
production requires a production WSGI server, private disk and backup policy.

Remaining: 2 sessions/checkpoints API; 3 JS adapter; 4 C Save/LOAD;
5 ranking API; 6 celebration/name/UI; 7 recovery; 8 final regression/docs.
