# Migration report — Web audio integration candidate

Repository: randomtconsultores/C-Space-2Web; branch migration/wasm.
Browser-playtested baseline remains e6cfff1ab0768dde831ab13ee999c7db20cba820.
Latest Codespace test/build baseline is a5a5f1b16019f855716a110104cde455f11f9486.

## User-reported validation update
User reports clean migration/wasm before pull, fast-forward to the exact a5a5f1b SHA, test and test-js passed, Wasm build passed, no errors. No server started. This validates repository tests/build including intro/transitions, not visual browser acceptance. The assistant did not operate Codespace or that browser.

## Current working increment: Web audio integration
Using the existing audio bridge and sound generator, the Web build now generates all 16 WAV files and their manifest. Howler maps C event IDs to those files and drains the bounded FIFO with an explicit eight-voice maximum.
The Web Controls menu toggles SOUND, which starts on. An original chiptune loops in the menu. Turning sound off, blurring the window or hiding the tab stops active sounds and clears queued events. web/main.c now emits the legacy player-damage, body-damage, reward-spawn, boss-entry and pickup events that were absent from the Web port. No native sources, gameplay rules, MAX_NIVEIS=4 or persistence are changed; no SoX parsing was added.
This working tree is based on 947ecbb. C/assets/JS tests passed, `make -B -f web/Makefile` passed, and the user confirmed audible browser playback after Howler preload was enabled. The local browser diagnostics expose WAV loads, C events, play calls, errors, volume and AudioContext state.

## Executed tests for this candidate
Assistant environment: isolated C11 units passed with Wall/Wextra/Werror, also ASan/UBSan. Covered all 16 mappings, system macro interception, unknown/null rejection, FIFO ordering, overflow without overwriting old events, wraparound, 10000 enqueue/take cycles, clear/reuse and unchanged rand sequence. Supporting terminal/runtime/framebuffer headers were locally transcribed and Git-blob-hash matched against retrieved repository versions. No complete checkout or Emscripten build in this environment; emcc unavailable.

## Earlier evidence retained
Original native game tested on user PC. Codespace native baseline make -B passed with 26 system-result warnings and one fread-result warning. Emscripten 6.0.13 and SDK Node v24.19.0 supported prior Web builds. User confirmed initial gameplay/menu/remapping and Enter/Escape/Q fixes; browser/version unrecorded. HEAPU8 fixed at 183017f, numpad Enter at a510b1e, Escape/Q at e6cfff1; PATH Node issue worked around with NODE="$EMSDK_NODE".

## Validation and limits
The current validation commands are:
```
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test test-js NODE="$EMSDK_NODE"
make -f web/Makefile
```
For real playback, serve port 8000, enable SOUND under Controls and trigger an existing game event such as taking damage or collecting a + pickup. The browser must be able to load Howler.js from its CDN; browser autoplay policy requires user interaction. Do not invent a player-fire sound: process_input has no such native request. Ranking celebration/audio stays deferred with persistence.
Visual intro/transitions, complete levels/collisions, console/network and final native regression remain pending. Persistence is last; LOAD/ranking are notices, bindings session-only, N debug pending. Fourth-level configuration/AI remains incomplete. Migration is not complete.
