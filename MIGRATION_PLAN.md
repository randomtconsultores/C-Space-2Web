# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Accepted in-game baseline: 5b5676235bd42092181b64a315cb9d80b1615dc8.
User confirmed intro, three configured levels/bosses, combat/collisions,
immunity, lives, score, progression/transitions, N, audio, touch and console/network.
Copilot reported C/Python/JS tests and forced Wasm build passed with a clean tree.
These accepted checks are not pending again.

## Server persistence: eight planned commits
1. Contract, SQLite schema/init and read-only API health (current candidate).
2. Anonymous sessions/protected checkpoint API: ownership, CSRF/origin checks,
   bounds, transactions and stale-write conflicts before exposing HTTP writes.
3. Async JS adapter, errors and stale-response handling.
4. Functional between-level Save/LOAD, preserving validated C state.
5. Nine-entry ranking API, stable ties and duplicate protection.
6. Nonblocking celebration, seven-character name capture and ranking UI.
7. Backup/restoration, conflicts, unavailable storage/API and fault tests.
8. Final native/Web/API regression, documentation and PR review before merge.

## Current limits
Only health/static HTTP routes and explicit CLI initialization are added now.
The game still has no working Save/LOAD/ranking. SQLite stays private on the
server; no browser SQLite, ORM, accounts, uploads, per-level history, persistent
preferences or anti-cheat. No native/Web/audio or running-server changes.
MAX_NIVEIS remains 4; missing fourth configuration stays explicit, not victory.
Final native regression is a closing check, not new gameplay implementation.
See server/CONTRACT.md for storage/schema/bounds and validation instructions.

Assistant implements/reviews/commits after approval. Copilot executes validation
commands/tests only. User accepts gameplay and controls merges.
