# Migration report — timed transition candidate

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Last user-validated browser baseline: e6cfff1ab0768dde831ab13ee999c7db20cba820.
Intro 8eb2506 is published but has no Codespace/browser validation result in this conversation.

## Changes
Added web/transition.h, web/transition.c and web/test_transition.c. Updated web/main.c, web/Makefile and migration documents.
Boss defeat now starts the original visual sequence automatically; completion advances exactly once without Enter. The final original level skips it. No gameplay updates during TRANSITION; begin_level clears input and preserves lives/score. The gameplay tick body is unchanged.
Native sources, native Makefile and MAX_NIVEIS=4 are unchanged. No persistence, audio or new level added.

## Tested and passed
- User PC: original native game.
- Codespace agent: native baseline make -B exit 0, 26 system return-value warnings and one fread warning.
- Agent/user through e6cfff1: C/JS tests and Wasm builds passed; private port 8000 returned HTTP 200; initial gameplay, menu/control capture, Escape and Q confirmed. Toolchain: Emscripten 6.0.13, SDK Node v24.19.0. Browser/version not recorded.
- Current transition units executed in assistant environment: C11 with Wall/Wextra/Werror; ASan/UBSan passed. Covered phase contents, nominal 7600/4600 ms durations, final blank hold, restart, invalid/backward/suspended timestamps, one-step catch-up bound, clipping and unchanged rand sequence. Isolated unit run with locally transcribed support fixtures, not a full checkout build.

## Prior failures addressed
HEAPU8 export fixed at 183017f; PATH Node execution worked around with NODE="$EMSDK_NODE"; numpad Enter fixed at a510b1e; Escape/Q separation fixed at e6cfff1. User confirmed the relevant browser corrections.

## Inspection only / not tested
Inspection: main-loop handoff, unchanged gameplay tick, input reset and preservation of player statistics.
Not tested for this candidate: complete Wasm build, real boss-to-transition progression, browser rendering/input/console, or native rebuild. No emcc in assistant environment. No new claim of intro validation.

## Reproduction in Codespace (repository root)
```
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test test-js NODE="$EMSDK_NODE"
make -f web/Makefile
make -f web/Makefile serve
```
Reuse the existing private port 8000 server if active. Browser check: defeat a boss; observe explosion/pause, first-level upgrade and stage-complete flashes; next level starts without Enter and retains lives/score. Input during the sequence must not skip it or leak into resumed play. Check suspension and console errors. Also validate the earlier intro.

## Remaining limits
Persistence is deferred until the end; LOAD/ranking still pending. Audio disabled, bindings session-only, N debug shortcut pending. Original A/D upgrade text retained even when controls are remapped. Fourth-level configuration/AI remains incomplete and is not treated as victory. Full level/collision/console audit and final native regression remain pending. Migration is not complete.
