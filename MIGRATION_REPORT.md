# Migration report — bounded audio bridge candidate

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Browser-playtested baseline remains e6cfff1ab0768dde831ab13ee999c7db20cba820.
Latest Codespace test/build baseline is a5a5f1b16019f855716a110104cde455f11f9486.

## User-reported validation update
User reports clean migration/wasm before pull, fast-forward to the exact a5a5f1b SHA, test and test-js passed, Wasm build passed, no errors. No server started. This validates repository tests/build including intro/transitions, not visual browser acceptance. The assistant did not operate Codespace or that browser.

## Current increment: audio boundary only
Added web/audio.h, web/audio.c and web/test_audio.c; updated web/platform.h, web/Makefile and migration documents.
The existing browser-only system macro now maps 16 exact SoX gameplay command literals from main.c, space.c and boss.c to stable numeric IDs. It never executes commands. Unknown/null input and queue overflow return -1; successful enqueue returns 0. Fixed FIFO capacity 16, drop-newest policy, explicit empty sentinel and clear operation. No allocation, blocking, shell, gameplay RNG or new gameplay rules.
Only take/clear are exported to the future JS consumer; command-string input is not exported. Native sources/build, MAX_NIVEIS=4, Web main/gameplay, browser.js and controls remain unchanged. Audio remains disabled and inaudible. This is preparation, NOT completed audio support.

## Executed tests for this candidate
Assistant environment: isolated C11 units passed with Wall/Wextra/Werror, also ASan/UBSan. Covered all 16 mappings, system macro interception, unknown/null rejection, FIFO ordering, overflow without overwriting old events, wraparound, 10000 enqueue/take cycles, clear/reuse and unchanged rand sequence. Supporting terminal/runtime/framebuffer headers were locally transcribed and Git-blob-hash matched against retrieved repository versions. No complete checkout or Emscripten build in this environment; emcc unavailable.

## Earlier evidence retained
Original native game tested on user PC. Codespace native baseline make -B passed with 26 system-result warnings and one fread-result warning. Emscripten 6.0.13 and SDK Node v24.19.0 supported prior Web builds. User confirmed initial gameplay/menu/remapping and Enter/Escape/Q fixes; browser/version unrecorded. HEAPU8 fixed at 183017f, numpad Enter at a510b1e, Escape/Q at e6cfff1; PATH Node issue worked around with NODE="$EMSDK_NODE".

## Next validation and limits
After publication, repeat test/test-js and Wasm build in Codespace. Existing commands:
```
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test test-js NODE="$EMSDK_NODE"
make -f web/Makefile
```
No new server is needed for this bridge-only check. Browser audio, SOUND toggle, activation, voice lifecycle and original main.c sound-site integration belong to the next increment. Ranking celebration/audio stays deferred with persistence. Do not invent a player-fire sound: process_input has no such native request.
Visual intro/transitions, complete levels/collisions, console/network and final native regression remain pending. Persistence is last; LOAD/ranking are notices, bindings session-only, N debug pending. Fourth-level configuration/AI remains incomplete. Migration is not complete.
