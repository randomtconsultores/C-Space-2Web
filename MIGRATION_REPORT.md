# Migration report - final state

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Last commit covered: 4e4929a. Native game unchanged (make / ./space).

## What the Web version does
- The original C core runs in WebAssembly with a bounded ASCII framebuffer,
  non-blocking menu, controls remapping, intro, level transitions, the N
  shortcut and generated audio.
- LOAD and SAVE use level checkpoints stored by a Flask + SQLite API with
  anonymous cookie sessions, CSRF and revisioned writes.
- A global top-9 ranking: HIGH SCORES page, name entry (1-7 chars, A-Z0-9) after
  a game over with score above zero, submission and position feedback.

## Architecture
- C owns state and rules. A small scalar bridge (web/persist_bridge.c) exposes
  request kind, ticket and fields; no strings or pointers cross the boundary.
- JavaScript (web/persistence_bridge.js, web/persistence.js) polls the bridge,
  performs one fetch at a time and completes the ticket. Late replies and Esc
  cancellation are discarded by ticket.
- Server (server/): Flask API, SQLite schema v2 (sessions, checkpoints, scores),
  migration only via init-db. Contract in server/CONTRACT.md.

## Evidence
Automated, run during review (not substitutes for browser acceptance):
- Backend: 41 tests (db, server, api, real HTTP smoke), including v1 to v2
  migration, ranking order, ties, global position above 9 and rate limit.
- C: make -f web/Makefile test, plus 37 bridge tests (with AddressSanitizer
  leak detection disabled in this environment).
- JS: 62 tests across bridge, client, browser input, wiring and N shortcut.
- Real HTTP check of the scores API with CSRF, Origin and rate limit.

Reported by the user from the browser over local HTTPS
(https://cspace.test:8443, Caddy + mkcert, Flask on 127.0.0.1:8002):
- LOAD, SAVE, F5 and LOAD restore level, lives and score.
- Score submission works, the position is shown after submitting, and both the
  checkpoint and the ranking survive a page refresh.
- The game stays usable with 3G throttling and offline in DevTools.

## Not verified
- Stopping the API while the page is already loaded (HTTP 502 on /api/v1) was not
  run; with Flask stopped, the Caddy setup serves nothing, including the page.
- Esc during a pending submission was not confirmed separately; the throttled
  and offline runs only show that the game stays usable.
- main.c has no automated tests; its behavior is accepted manually.
- Public deployment is not done: backup/recovery, session maintenance and
  production configuration are out of this report.

## Issues found and fixed during acceptance
- GitHub Codespaces rewrites Origin on forwarded ports (proven with an echo
  server); acceptance moved to local HTTPS (683afbc).
- Name entry could not confirm or cancel because Enter, Backspace and Escape
  were not forwarded from the browser; fixed before publishing the final patch.
- Submission cleared the name before reading it (4e4929a).

## Security notes
mkcert private keys, certificates, SQLite files and ZIPs are never committed
(.gitignore in docs/local-https). A key that was shared outside the machine
should be regenerated with mkcert.
