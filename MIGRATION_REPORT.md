# Migration report — isolated persistence client (3/8)

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Client commit: 5610a4f8fa41a9c0ded87e5d910a37ca87ddaee1.
Base: 2950a1f613c13f9f2b2a1854e85145218da1c900.

## Accepted evidence
Game baseline 5b567623: user accepted intro, configured levels/bosses, collisions,
immunity, lives/score, transitions/progression, N, audio and touch.
SQLite/health 9e2a9e6: 13 tests and real loopback HTTP accepted; missing storage
was not created by requests; integrity/FKs passed after explicit temporary init.
API core a79e276 and resource cleanup aa12b2f: first Codespace run was 22/23 with
no warnings. The JSON test assumed a parser recursion limit that was not portable.
2950a1f corrected that fixture without weakening the API. User/Copilot then
reported 23/23 passed, no warnings, clean/synchronized Git status. Codespace
versions previously reported: Python 3.14.2, SQLite 3.45.1, Flask 3.1.3.
Real HTTP session/checkpoint smoke test has not been reported; remains pending.

## Published scope
5610a4f adds web/persistence.js and web/test_persistence.js. This follow-up updates
only roadmap/report. No C, browser.js, index.html, Makefile, server code/schema or
existing build assets change. The adapter is not loaded/copied by the game build
yet; no automatic requests occur on script import. Game Save/LOAD/ranking remain
unconnected. The initial combined code/docs push failed with connector HTTP 400;
HEAD was checked unchanged. The separately approved two-file code push succeeded.

API: SpacePersistence.createClient() in a classic script, or CommonJS exports.
Client methods: load(), save(snapshot), cancel(), getState(). load returns null
only for checkpoint_missing, not disabled API. save requires successful load;
caller supplies exactly format_version/level_index/lives/score, never owner or
revision. Returned checkpoints and state are immutable copies. Level 3 is preserved.
Requests use fixed relative endpoints, same-origin credentials/mode, no-store,
redirect rejection and in-memory CSRF only. Cookie handling stays with the browser.
Default whole-operation deadline: 10 seconds. Response UTF-8 JSON is bounded to
4096 bytes and 4097 stream reads. Overlapping operations reject busy; no queues
or automatic write retries. Cancellation rejects promptly and ignores late replies.
Errors expose sanitized code/status/uncertain, not raw server or transport text.
After failures the known revision is invalidated; an explicit load is required
before another save. A possibly dispatched write may still commit after abort:
uncertain=true means reconcile, not retry or assume rollback. Stage 4 must also
check the UI/game generation before applying any already-resolved load result.

## Executed and pending
node --check web/persistence.js and node --test web/test_persistence.js passed
locally: 13/13 tests on Node v20.20.1, no stderr warnings. Tests use injected fetch
and deterministic timers with native Response/ReadableStream, not real HTTP or
browser cookies. They cover lifecycle/revisions, security options, input/response
bounds, HTTP errors, uncertainty, cancellation/timeouts, stalled bodies and late
replies. This does not claim browser/C/HTTPS integration or backend HTTP acceptance.
Copilot should rerun the new Node suite in the existing validated clone/venv after
an approved fast-forward, without changing code or repeating accepted playtests.
Codespace validation of the new client is pending, not claimed as passed.

## Workspace discipline
User confirmed old tmp.3gjWI6vRHR clone/venv and its parent removed. Reuse the clean
validated clone under tmp.PEzCB8T8me; create no more clones. Keep original checkout
at 5b567623 and its five local modifications intact. No real databases or listeners
were created for these Node tests. Do not touch port 8000 or use broad temp cleanup.
Remaining stages: C/build integration (4), ranking (5), celebration/UI (6), recovery
(7), final regression/docs (8). Persistence is not complete; merges remain user-owned.
