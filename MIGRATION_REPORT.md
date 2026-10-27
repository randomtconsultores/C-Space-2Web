# Migration report — N keyboard integration candidate

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
This candidate builds on C shortcut commit 5d9a4036515e899371bef13d53bbc75690ab3635.

## Accepted audio baseline
Audio at c5fe243cce653cf7fc1325355660abebf7e73f0d was confirmed in game by the
user on 2026-10-27. This includes the requested menu music and SOUND initially
ON. Do not treat audio implementation as pending. Earlier C/assets/JS tests and
forced Wasm build were reported passed in Codespace for the matching audio code.
This is scoped acceptance, not a full progression/collision audit.

## Current increment
browser.js extends the existing held counts to 11 and maps N/n to index 10.
No parallel input system is introduced. Existing control capture still handles
reserved N, modifiers are ignored, repeated keydowns do not emit extra presses,
and focus reset clears the held state. The status line explains N/Q.
web/test_n_shortcut.js is added to make test-js. Existing audio/input tests remain
unchanged. See web/N_SHORTCUT.md for C semantics and browser acceptance steps.

## Evidence and limitations
- C stage: isolated runtime tests passed locally with Wall/Wextra/Werror and
  ASan/UBSan. Controls/framebuffer/terminal matched repository blob hashes;
  space.h/globals were local fixtures. Main routing was inspected only.
- This JS stage: node --check, existing browser regression and focused N tests
  executed and passed with mock DOM. They are not real browser playtests.
- Full Wasm build, native rebuild and N browser acceptance were NOT executed
  by this assistant. Copilot/Codespace validation is still required.
- Prior SDK baseline: Emscripten 6.0.13, SDK Node v24.19.0. Earlier native make -B
  passed with 26 system-result warnings and one fread-result warning.

## Reproduction
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test test-js NODE="$EMSDK_NODE"
make -B -f web/Makefile

Do not discard local changes when updating; reuse the existing private server.
MAX_NIVEIS=4, sound defaults, native gameplay files and persistence are unchanged.
The incomplete fourth level remains an inherited error. Full boss/collision,
progression, intro/transition, touch, console/network and final native regression
checks remain pending where not explicitly validated. Persistence is last;
LOAD/ranking/celebration remain deferred. Migration is not complete.

Assistant owns implementation/review/commits; Copilot performs validation only;
user owns in-game acceptance and merges.
