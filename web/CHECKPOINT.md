# C checkpoint foundation — stage 4a/8

Base: a21a43ca1729e79288b87e1614a85acdd83f3c39, migration/wasm.
This block adds a logical C DTO, validation/restore helpers and local LOAD tickets.
It does NOT wire Save/LOAD into main.c, menus, JS, build exports or the browser.
No existing source, backend, database, audio/control or game behavior is changed.
Stage 4 remains incomplete until the C/JS/UI bridge and its integration tests land.

## Contract
WebCheckpoint carries only format_version=1, next level_index=0..3, lives=1..9
and score=0..2147473647. long follows Player internally; never persist sizeof this
struct, padding or a Player memory dump. The future Wasm bridge must pass validated
scalar fields, not raw pointers or native layouts. The backend owns revisions.
Capture validates and copies, never mutates Player. Game Over cannot be captured.
A DTO for level 3 is valid progress even while that level is unconfigured.
Restore first validates DTO/configuration. Invalid/unavailable returns leave both
Player output and level output byte-for-byte unchanged. Success builds a fresh
Player (spawn 2,7; no immunity/projectiles) and the next 10000-point reward threshold.
The score cap keeps that threshold within signed 32-bit range.

WebCheckpointGate is zero-initialized local lifecycle state, not authentication.
Begin rejects overlap. Cancel invalidates pending work. Consume requires the exact
positive ticket once. Tickets never wrap: exhaustion blocks new requests.
Stage 4b must consume the ticket before applying a callback, restore into temporary
outputs, then apply only on success. begin_level currently resets reward threshold;
the bridge must preserve the validated derived threshold after initializing a LOAD.
It must cancel pending work when leaving the LOAD context or starting another game.

## Executed validation
Standalone native C tests passed: five groups, strict C11 warnings as errors,
plus a second run with AddressSanitizer/UndefinedBehaviorSanitizer, no diagnostics.
Compiler: GCC 15.2.0. Original space.h/levels.h blobs were checked; local levels.c
was transcribed for this test with the published three-level configuration. Its
whole-file blob was not verified. Re-run against the fetched originals in Codespace.
No Emscripten is installed here: no Wasm/full-game integration success is claimed.
The tests exercise bounds, failure preservation, capture, reward thresholds,
unsupported fourth level, nulls, stale/duplicate tickets and counter exhaustion.

Run from repository root with these sources present:
cc -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. web/checkpoint.c levels.c web/test_checkpoint.c -lm -o /path/to/disposable/checkpoint-test
/path/to/disposable/checkpoint-test

Copilot validates/executes commands only. Do not create clones, modify the original
checkout, initialize real storage or start listeners. For fetched-only validation,
extract just these sources/headers with git show into a disposable test directory
and remove its files/binary afterwards; do not merge/reset/stash local changes.

## Existing accepted/pending evidence
User/Copilot confirmed backend 23/23 at 2950a1f and JS 13/13 at a21a43c
(Node v24.21.0, both syntax checks passed, temporary files removed).
Backend real HTTP session/checkpoint smoke remains pending. It and browser HTTPS/
cookie validation gate activation of the final Save/LOAD flow, not this inert module.
Original game acceptance and the five local changes remain protected. Next blocks:
C scalar bridge/lifecycle, nonblocking browser/menu/transition flow, build wiring
and focused integrated tests. Root roadmap/report still describe stage 3 until the
next integration documentation update; this file records the scope of stage 4a.
