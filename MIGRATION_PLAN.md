# Migration plan

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Goal: run the original C game in the browser through WebAssembly, keeping the C
core (rules, state, levels, bosses) and replacing only what depends on the native
environment (terminal, input, audio, files). Merges remain user-owned.

## Status: migration complete (stages 1-7 done, stage 8 optional)

| # | Stage | Commits | State |
|---|-------|---------|-------|
| 0 | Game baseline in the browser: framebuffer, loop, menu, controls, intro, transitions, N shortcut, audio | 3f34f8d .. 5b56762 | Accepted by the user |
| 1 | SQLite foundation and read-only health API | 9e2a9e6 | Accepted |
| 2 | Opt-in sessions and revisioned checkpoints | a79e276, aa12b2f, 2950a1f | Accepted (23/23 tests then) |
| 3 | Isolated asynchronous JS client | 5610a4f, a21a43c | Accepted |
| 4 | C checkpoint, scalar bridge, JS bridge, build wiring, HTTP smoke test | 648a659, d4a80d1, d0dfb7d, b03fd15, 075d8ad | Accepted |
| 5 | LOAD menu connected to the bridge | 9ea6701 | Accepted |
| 6 | SAVE of level checkpoints, bridge availability | 2b72992 | Accepted in browser |
| 6b | Local HTTPS acceptance setup (Caddy + mkcert) | 683afbc | Used for browser acceptance |
| 7a | Global ranking API and schema v2 migration | aa3a6f7 | Accepted (41 backend tests) |
| 7b | HIGH SCORES page and read bridge | a969fe9 | Accepted in browser |
| 7c | Name entry and score submission | 9d0d89c, 4e4929a | Accepted in browser |
| 8 | Optional: main.c frame coverage, level 4 without configuration, cleanup | - | Not done |

Commit 9d0d89c shipped with a defect: the name was cleared before the
submission, so no request was sent. 4e4929a fixed it (web_persist_name_submit).

## Decisions kept
- MAX_NIVEIS=4; the unavailable fourth level is not victory.
- No browser database, ORM, accounts, uploads, arbitrary asset paths,
  persistent preferences or anti-cheat. Sessions are anonymous cookie sessions.
- Origin, Fetch Metadata and CSRF checks are never relaxed.
- Migrations run only through the init-db command, never inside a request.
- Ranking: POST/GET /api/v1/scores; name 1-7 chars A-Z0-9; score 0..2147473647;
  order by score desc, created_at asc, id; GET returns at most 9; POST returns the
  real global position; the client shows NEW HIGH SCORE when position <= 9.
  Rate limit 5 submissions per 60 s per session. See server/CONTRACT.md.
- Scores are stored globally; the link to the session is optional (ON DELETE SET NULL).

## Roadmap with commits
1. Browser baseline: 3f34f8d, d918b54, 183017f, a510b1e, 426096a, 916994c,
   e6cfff1, 8eb2506, a5a5f1b, 5b56762, 1e9ed46, 8b86c81, c5fe243.
2. Persistence server: 9e2a9e6, a79e276, aa12b2f, 2950a1f.
3. Persistence client and bridge: 5610a4f, 648a659, d4a80d1, d0dfb7d, b03fd15, 075d8ad.
4. Save/LOAD in the game: 9ea6701, 2b72992, 683afbc.
5. Global ranking: aa3a6f7, a969fe9, 9d0d89c, 4e4929a.
6. Next (optional): automated coverage of main.c frame logic; configure or remove
   level 4; backup/recovery and session maintenance for a public deployment;
   final native/Web/API regression and PR review.

## Operational notes
- A database created before aa3a6f7 is schema v1. Run once:
  SPACE_DATABASE=<path> server/.venv/bin/flask --app server.app init-db
  Until then, the scores API answers 503 storage_unavailable.
- Browser acceptance needs HTTPS with a stable Origin: use docs/local-https.
  A GitHub Codespaces relay rewrites Origin to http://localhost, so it cannot
  be used for browser acceptance of POST requests.
- Generated files in web/build are never edited by hand.
