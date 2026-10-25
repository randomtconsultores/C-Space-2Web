# Migration report — playable browser increment

## Preserved
Original space.c, levels.c, boss.c, draw.c, native main.c/menu.c and native Makefile remain unchanged. MAX_NIVEIS stays 4. JavaScript does not implement game rules.

## Changes
- Web-only terminal adapter maps existing drawing calls to the bounded ASCII framebuffer and getch to a finite per-tick input snapshot.
- Persistent C state, browser main loop, fixed updates, independent rendering and focus/input reset.
- W/S movement; A/D from stage 2; Space fires once per press; Enter starts/restarts/continues. Movement uses fixed updates, not OS repeat.
- Original gameplay orchestrator ported from main.c. Adjacent reward rows and negative hit-sentinel coordinates are bounds-checked in the Web orchestrator; this prevents inherited invalid accesses without editing original logic modules. Zero-initialized player/boss state.
- Missing level configuration stops with an explicit error, not victory. MAX_NIVEIS is not reduced.

## Evidence
- Native gameplay: user confirmed original game on PC.
- Native baseline: Codespace agent reported make -B exit 0, 26 system warnings and one fread warning.
- Previous commit 3f34f8d: Codespace agent reported framebuffer tests and Wasm build passed with emcc 6.0.13; port 8000 server started. The supplied external HTML was Codespaces authentication, not the game. Browser rendering remains unconfirmed.
- Current increment, assistant environment: runtime/input/timing/formatter unit test passed with Wall/Wextra/Werror and ASan/UBSan; JavaScript syntax and mock-DOM tests passed.
- Validated only by inspection: original module linkage and gameplay orchestration.
- Not tested: current Wasm build, native rebuild, real browser rendering/input/collisions/progression, browser console, HTTP resource loading or persistence. No Emscripten in assistant environment.

## Commands (repository root)
Activate the existing SDK in each new terminal: source "$HOME/emsdk/emsdk_env.sh".
```
make -f web/Makefile test test-js
make -f web/Makefile
make -f web/Makefile serve
```
Use Emscripten 6.0.13 for reproduction. Server serves only web/build on port 8000; keep forwarding private. Python 3 required; Apache not required. If a server is already running, reuse it; no second listener is needed.

## Limits
Menus/remapping, intro/transition animations, Q/N shortcuts, audio and persistence are pending. SoX calls are disabled only in Web builds; page labels missing audio/save support. Original blocking draw animations are not called by the Web entry.
Stage transitions currently wait for Enter. Boss blink follows render frames (visual only). Existing fourth-level configuration/AI incompleteness remains; error screen is not completion.
Migration is incomplete; no browser playtest success is claimed.
