# Migration report — persistence foundation candidate (1/8)

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Base: 5b5676235bd42092181b64a315cb9d80b1615dc8.

## Accepted evidence
User confirmed intro, three configured levels/bosses, combat/collisions,
immunity, lives, score, progression/transitions, N, audio, touch and console/network.
Copilot reported C/Python/JS tests and forced Wasm build passed at that SHA,
with clean Git status. Menu music and default SOUND ON are accepted.
These gameplay checks are not being reclassified as pending.

## Candidate
New server package with Flask app factory, explicit transactional SQLite init,
versioned sessions/checkpoints schema, read-only health and bounded static file
serving. Missing/foreign/incompatible storage is refused, not reset by requests.
Database path must be absolute and outside WEB_ROOT; new POSIX files use 0600.
Only init-db creates storage. HTTP session/checkpoint/ranking writes are absent;
phase 2 must add their protections before exposing them. No native game files,
Wasm build, browser JS, audio or running-server changes are included.

## Verification actually executed here
Python 3.12.8 / SQLite 3.46.1: eight database tests passed with temporary files.
Coverage: idempotent init, noncreating reads, constraints/foreign keys, rejection
without resetting future/foreign databases, DDL rollback, concurrent init and
URI characters/new-file permissions. All Python files passed AST parsing.
Flask is not installed in the assistant environment: five Flask/HTTP tests were
written but NOT executed here. No listening HTTP server, Codespace integration,
browser test or native/Wasm build was run for this candidate.

## Codespace validation after publication
Use the isolated environment and run both test modules in server/CONTRACT.md.
Report Python/SQLite/Flask versions and test results. Do not change code, initialize
a real database, or start/stop the current port 8000 server for this validation.
An actual network smoke test remains pending after the Flask tests pass.

## Remaining
Persistence is not complete: LOAD/ranking are unconnected. Sessions/security,
checkpoint routes, JS/C integration, ranking, celebration, backup/recovery and
production deployment checks follow in commits 2..8. WAL is not enabled here.
Disk, secrets, existing permissions and backups require explicit operator policy.
MAX_NIVEIS=4 and the missing fourth configuration are unchanged. Final native
regression remains for closure. Copilot validates; assistant codes/commits;
user owns in-game acceptance and merges.
