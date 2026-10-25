# Migration report — first browser increment

## Changes
- space.h: suppress ncurses inclusion only when SPACE_WEB is explicitly defined.
- web/: separate build, HTML/JS host, bounded 40x15 C framebuffer, map preview and framebuffer test.
- Original levels.c and native Makefile remain unchanged. MAX_NIVEIS remains 4.

## Commands
Run from repository root after activating Emscripten:
```
make -f web/Makefile test
make -f web/Makefile
make -f web/Makefile serve
```
The last command serves only web/build on port 8000. Keep Codespaces port visibility private; stop with Ctrl+C. Python 3 is required for this temporary HTTP server; Apache is not required for this increment.
Open index.html through the forwarded HTTP address, not file://.

## Evidence
- User PC: original native gameplay confirmed by user before repository creation.
- Codespace agent, relayed by user: baseline native build in temporary git archive copy, make -B exit 0; 26 system warnings and one fread warning.
- Assistant execution environment: new framebuffer test passed with C11, Wall/Wextra/Werror; browser.js syntax check passed with Node. Framebuffer also passed AddressSanitizer/UndefinedBehaviorSanitizer.
- Assistant execution environment: toolchain guard correctly failed because emcc is absent. This is NOT a successful Wasm build.
- Additional limited check: native preview smoke compilation passed using manually transcribed level source/header; this is not an exact-checkout or Wasm build. Make dry run passed after adding these temporary sources.
- Validated only by inspection: level generator reuse, HTML integration and Web build configuration.
- Not tested: Emscripten compilation, HTTP resource loading, browser rendering or console, native rebuild after this header edit.

## Limits
Static first-level map only: no player input, simulation, menus, audio or persistence yet. The page labels this explicitly. Migration is not complete. No browser success is claimed.
No root executable, saves or scores were modified. No generated build artifacts are tracked.
