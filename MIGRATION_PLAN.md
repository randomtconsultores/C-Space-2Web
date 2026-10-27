# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Native baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.
Last user-validated browser baseline: e6cfff1ab0768dde831ab13ee999c7db20cba820.
Last Codespace C/JS-test and Wasm-build baseline: a5a5f1b16019f855716a110104cde455f11f9486.

## Completed and pending acceptance
- Wasm/HTTP/framebuffer, initial gameplay, menu, remapping, Enter variants, Escape and Q were checked through the user. Not a full gameplay audit.
- Binary intro and timed stage transitions are implemented. User reports test/test-js and Wasm build passed for a5a5f1b, with no errors; no server was started in that check.
- Visual intro/transitions, input isolation and complete progression still need browser acceptance.

## Current candidate: Web audio integration
- Keep native gameplay files unchanged. The Web adapter maps the 16 existing command literals to stable IDs and never invokes a shell.
- Generate the existing replaceable WAV assets during the Web build. Howler consumes the bounded C FIFO; playback is capped at eight simultaneous voices.
- SOUND starts on and can be toggled in Controls. An original chiptune loop plays in the menu; disabling sound or losing focus stops current voices and clears queued events.
- Restore only the missing legacy sound sites in web/main.c. The generated menu loop is the only new music; no new gameplay effects, native gameplay changes or persistence are included.
- C/assets/JS tests and forced Wasm rebuild passed. User confirmed audible playback in the browser.

## Remaining order
1. Validate the Web audio integration in Codespace and test real playback in a browser with Howler CDN access.
2. Validate intro/transitions visually and audit bosses, collisions, configured progression, suspension and console/network behavior.
3. Persistence LAST: original saves/ranking, ranking celebration/name capture, reload/error tests. Ranking sound is deferred with that feature. SQLite and future resource/data evolution remain design considerations.
4. Final native/Web regression, documentation and PR review before merge.

## Constraints
MAX_NIVEIS remains 4; the incomplete fourth configuration is an explicit inherited error, not early victory. No new levels, accounts or assets. Native build/core remain unchanged. LOAD/ranking stay notices. User operates Codespace/browser and controls merges; agent runs short checks.
