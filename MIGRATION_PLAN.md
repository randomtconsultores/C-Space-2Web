# Migration plan

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Accepted game baseline: 5b567623. Intro, configured levels/bosses, collisions,
immunity, lives/score, transitions, N, audio and touch were accepted by the user.
Those playtests are not pending again.

## Persistence roadmap
1. SQLite/health: 9e2a9e6 accepted, including real loopback HTTP and integrity/FKs.
2. Sessions/checkpoints: core a79e276, cleanup aa12b2f, deterministic tests 2950a1f.
   User/Copilot confirmed 23/23 tests, no warnings, clean checkout at 2950a1f.
   Real HTTP session/checkpoint smoke test remains pending; not a public deployment.
3. Isolated JS adapter: published as 5610a4f8fa41a9c0ded87e5d910a37ca87ddaee1.
   Thirteen local Node tests passed; Codespace rerun pending. No loading,
   build wiring or C/UI activation yet. Validation command:
   node --test web/test_persistence.js
4. Wire build/browser/C Save/LOAD between levels, preserving valid C state.
   Gate activation on backend HTTP smoke and browser HTTPS/cookie validation.
5. Nine-entry ranking API, stable ties and duplicate protection.
6. Nonblocking celebration, seven-character name and ranking UI.
7. Backup/recovery, session maintenance, conflicts and failure/deployment tests.
8. Final native/Web/API regression, docs and PR review; user controls merges.

Keep MAX_NIVEIS=4: unavailable fourth level is not victory. Preserve audio,
controls and current gameplay. No browser DB, ORM, accounts, uploads, arbitrary
asset paths, persistent preferences or anti-cheat. See server/CONTRACT.md.
Assistant implements/commits after approval; Copilot validates/executes commands.
Reuse the validated clone/venv under tmp.PEzCB8T8me; no new clones. The user
reported the old tmp.3gjWI6vRHR clone/venv/parent removed. Preserve the original
checkout at 5b567623 with its five local changes. Do not touch port 8000.
