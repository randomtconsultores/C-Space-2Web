# Migration report — checkpoint API and resource cleanup (2/8)

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
API core published as a79e2760325757c3e77bad2dfd117fe7f3bdf3f0.

## Accepted baselines
Game at 5b567623: user confirmed intro, configured levels/bosses, collisions,
immunity, lives/score, progression/transitions, N, audio, touch and console/network.
Foundation 9e2a9e6: user/Copilot reported 13 tests passed on Python 3.14.2,
SQLite 3.45.1, Flask 3.1.3. Real loopback HTTP returned 503 without creating
storage, then 200/no-store after explicit init; integrity_check=ok and FK check
empty. Temporary listener/connection were closed; old checkout/8000 untouched.

## Current scope
Opt-in anonymous sessions, SHA-256 token storage, secure host cookie, session-bound
HMAC CSRF, exact configured Origin/fetch metadata checks, strict bounded JSON,
owner-only checkpoints, transactional revision conflicts and session quotas.
No client owner/SQL/path is accepted. Session expiry does not delete stored rows.
Without origin+secret, routes remain absent. This is not yet game Save/LOAD or a
production-ready public service; HTTPS/private disk/backups/upstream limits matter.

## Follow-up changes
Tests now explicitly close file-backed HTTP responses and raw SQLite fixture
connections. The foreign-database fixture commits before closing, preserving
its test semantics. Production db.py/schema are unchanged and blob-verified.
Default-server tests force disabled API configuration to avoid ambient secrets.
Contract/roadmap now describe stage 2 and its pending acceptance accurately.

## Executed versus pending
During API preparation: syntax and isolated stdlib origin/HMAC/strict-JSON helper
checks passed, not HTTP tests. For this cleanup: all Python files parsed; the
8 updated database tests passed locally with tracemalloc/ResourceWarning enabled
on Python 3.12.8 / SQLite 3.46.1, with no warnings reported in that run.
Flask is absent locally. The 5 updated HTTP regression tests and 10 API tests
were NOT run here. No claim is made that the reported warning is resolved on
Python 3.14 until Codespace confirms the complete suite. Real session/checkpoint
HTTP smoke test is also pending. Do not enable public writes before acceptance.

## Next validation
Run the 23-test command in server/CONTRACT.md in a clean clone/venv. Report full
results/warnings and versions; do not edit code, print real credentials, initialize
real storage or start/stop port 8000. Failures are fixed by the assistant.
Native/Wasm/browser code, MAX_NIVEIS=4 and accepted audio defaults are unchanged.
Remaining commits 3..8 cover JS/C integration, ranking/celebration, recovery and
final regression. Persistence is not complete. User controls merges.
