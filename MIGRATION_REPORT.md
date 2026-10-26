# Migration report — binary intro candidate

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Last browser-validated baseline: e6cfff1ab0768dde831ab13ee999c7db20cba820.
Codespace results below were supplied by the user; this assistant did not operate that browser.

## This increment
Created web/intro.h, web/intro.c and web/test_intro.c. Updated Web entry/build and migration documents.
PLAY enters a timed INTRO state before new_game. It reuses the original draw.c column/title algorithm: 80 frames, 40 ms each, title after frame 35. Gameplay does not run during INTRO. new_game resets C input and seeds/generates the map only after completion.
There is no new skip control. Native draw.c and other original gameplay modules, native Makefile and MAX_NIVEIS=4 are unchanged.

## Tested and passed
- User PC: original native gameplay before repository creation.
- Codespace agent: native baseline make -B exit 0, with 26 system return-value warnings and one fread return-value warning.
- Codespace agent/user: Wasm builds and C/JS tests through e6cfff1 passed; private port 8000 returned HTTP 200. Emscripten 6.0.13; SDK Node v24.19.0.
- User: initial gameplay, menu navigation, control capture/cancel and Escape/Q correction confirmed in the browser. Browser/version not recorded. This is not a full test of all configured levels or collision cases.
- Current candidate, assistant environment: isolated C intro tests passed with Wall/Wextra/Werror, also under ASan/UBSan. Covered 3200 ms nominal completion, final-frame hold, title timing, restart, invalid/backward/suspended timestamps and bounded catch-up. Supporting headers/framebuffer/terminal were locally transcribed fixtures, not a complete checkout build.

## Earlier tested failures and corrections
- HEAPU8 was not exported: fixed at 183017f; user subsequently confirmed rendering.
- PATH node could not execute: tests passed using NODE="$EMSDK_NODE", without global permission changes.
- Numpad Enter was unmapped: fixed at a510b1e and confirmed by the user.
- Q had been used for submenu return: e6cfff1 restores Q damage and uses Escape for submenus, confirmed by the user.

## Inspection only / not tested
- Inspection only: INTRO routing and unchanged gameplay update body in the Web entry; no simulation ticks while INTRO is active.
- Not tested for this candidate: full Wasm build, actual browser animation, input isolation at transition or play after intro. No Emscripten in the assistant environment.
- Still pending: level-transition animations, audio, full progression/collision and console/network audit, final native rebuild, and persistence/reload/error testing.

## Commands (repository root)
```
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test test-js NODE="$EMSDK_NODE"
make -f web/Makefile
make -f web/Makefile serve
```
Use the existing SDK/server; do not open a second listener if port 8000 is already serving web/build. Keep forwarding private. Open through HTTP, not file://.
Browser acceptance: PLAY shows the binary intro/title for about 3.2 active seconds, then starts normally; Enter/fire/Q during the intro must not skip it or leak into the new game. Check repeated PLAY and returning after a suspended tab.

## Remaining limits and decisions
Persistence is deferred until the end of migration by user decision. LOAD GAME/HIGH SCORES remain notices; SQLite is under consideration, not implemented. No logins, new assets or per-level history added.
Audio remains disabled; remappings are session-only. Stage transitions currently require Enter. The missing fourth-level configuration/AI is inherited and is reported without declaring victory.
Migration remains incomplete. This intro has NOT yet been validated in the Codespace browser.
