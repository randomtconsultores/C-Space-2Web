# Persistence API v1 — commit 2/8

Core API: a79e2760325757c3e77bad2dfd117fe7f3bdf3f0. This follow-up closes test
resources and documents the API. No C/JS game integration, ranking, real database
initialization or running-server changes are included. SQLite schema remains v1.

## Configuration
SPACE_API_ORIGIN and SPACE_API_SECRET both empty: session/checkpoint routes are
not registered (404/405). Partial or invalid configuration refuses startup.
Origin must be canonical HTTPS, with no path, credentials, trailing slash or
explicit default port. Secret must be private/random and at least 32 bytes.
No origin is inferred from Host or forwarded headers. Do not print real secrets,
cookies or CSRF values. Real browser deployment requires HTTPS termination and
same-origin frontend/API hosting; do not weaken cookies for testing.
Existing SPACE_DATABASE/SPACE_WEB_ROOT absolute/private-path rules still apply.
Requests never create or migrate storage. init-db remains an explicit CLI action.

## Endpoints
POST /api/v1/session requires exact Origin and an empty JSON object. It creates
(201) or resumes (200) an anonymous session and returns csrf_token/expires_at.
Cookie __Host-space_session is Secure, HttpOnly, SameSite=Strict, Path=/ and has
no Domain. Only the SHA-256 token hash is stored in SQLite; no owner ID is returned.
GET /api/v1/checkpoint requires that cookie and X-CSRF-Token. GET Origin may be
absent but must match when supplied. PUT also requires exact Origin. Fetch
metadata, when present, must say same-origin. No CORS access is granted.

PUT fields: format_version=1, level_index=0..3, lives=1..9,
score=0..2147473647, expected_revision=0..2147483646. Only integers, not booleans
or floats, are accepted. Unknown fields/queries, duplicate JSON keys and non-finite
numbers are rejected. JSON is UTF-8 and limited to 4096 bytes.
Revision 0 creates; later writes must match the server revision. A BEGIN IMMEDIATE
transaction serializes comparison/write. Stale writes return 409 without changing
the save. Checkpoint responses add server-owned revision/updated_at, not owner IDs.
All API responses use no-store. Cookie/Origin/fetch metadata are in Vary.

## Errors and lifecycle
401 distinguishes missing, invalid and expired sessions; 403 origin/CSRF rejection;
404 missing checkpoint; 409 revision conflict; 422 invalid fields/query;
429 session capacity/creation rate; 503 unavailable storage. HTTP JSON errors use
structured codes. Invalid/expired cookies are cleared, stored saves are not deleted.
Default fixed session life is 30 days, capacity 10000 total stored sessions and
creation limit 30/minute globally. Resume does not extend server expiry. Expired
rows still count toward capacity; maintenance/recovery is explicit later work.
Anonymous cookie loss/expiry has no account recovery. This is not anti-cheat.
Public deployment still needs private disk, backups and upstream rate limits.
Level 3 may be saved although currently unavailable: C integration must preserve
that save and report unavailable, never victory. No arbitrary asset path is accepted.

## Validation only, in a clean clone
python3 -m venv server/.venv
server/.venv/bin/python -m pip install -r server/requirements.txt
server/.venv/bin/python -X tracemalloc=10 -W always::ResourceWarning -B -m unittest server.test_db server.test_server server.test_api -v

Expected suite: 8 SQLite, 5 health/static regression and 10 API tests (23 total).
All use temporary storage and test_client, not public listeners. Tests override
API configuration explicitly; never configure real secrets just to run them.
Do not touch port 8000, the old checkout or existing databases. Report versions,
results, warnings and Git status; do not modify code to pass validation.
The assistant re-ran the 8 updated SQLite tests on Python 3.12.8/SQLite 3.46.1.
Flask is unavailable there: the 15 HTTP tests and disappearance of the reported
HTTP/file warning require Codespace confirmation. No claim of HTTP success yet.
After that, an isolated real HTTP session/checkpoint smoke test is still required.

Next: 3 JS adapter; 4 C Save/LOAD; 5 ranking API; 6 celebration/name/UI;
7 recovery; 8 final regression/docs. Native and accepted gameplay remain unchanged.
