# Migration plan

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Accepted game baseline: 5b567623. User validated intro, three configured levels,
bosses, collisions/immunity, lives/score, transitions/progression, N, audio,
touch controls and console/network. Those playtests are not pending again.

## Persistence roadmap
1. SQLite foundation/health: 9e2a9e6 validated in Codespace (13 tests) and real
   loopback HTTP, including no creation before init and integrity/FK checks.
2. Sessions/checkpoints: core a79e276 published; resource-cleanup/docs follow-up
   prepared. Eight updated SQL tests passed locally. Fifteen HTTP tests and real
   checkpoint HTTP smoke test require Codespace validation before acceptance.
3. Async JS adapter, error/cancellation and stale-response handling.
4. Functional between-level Save/LOAD without erasing validated C state.
5. Nine-entry ranking API, stable ties and duplicate protection.
6. Nonblocking celebration, seven-character name capture and ranking UI.
7. Backup/recovery, lifecycle cleanup, conflicts and fault/deployment tests.
8. Final native/Web/API regression, documentation and PR review before merge.

## Boundaries
Server SQLite only, no browser database/ORM/accounts/uploads/per-level history,
persistent preferences or anti-cheat. API is opt-in with configured HTTPS origin
and secret. No current game, port 8000 or real storage is changed by these commits.
LOAD/ranking remain unconnected. Keep MAX_NIVEIS=4; unavailable fourth level is
explicit, not victory. Existing generated music and SOUND defaults are retained.
See server/CONTRACT.md for API limits, session expiry and validation commands.

Assistant implements/reviews/commits after approval. Copilot executes validations
and commands only. User owns gameplay acceptance and merges.
