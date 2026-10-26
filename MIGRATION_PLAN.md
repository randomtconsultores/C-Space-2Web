# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Native baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.
Last user-validated browser baseline: e6cfff1ab0768dde831ab13ee999c7db20cba820.
Last Codespace C/JS-test and Wasm-build baseline: a5a5f1b16019f855716a110104cde455f11f9486.

## Completed and pending acceptance
- Wasm/HTTP/framebuffer, initial gameplay, menu, remapping, Enter variants, Escape and Q were checked through the user. Not a full gameplay audit.
- Binary intro and timed stage transitions are implemented. User reports test/test-js and Wasm build passed for a5a5f1b, with no errors; no server was started in that check.
- Visual intro/transitions, input isolation and complete progression still need browser acceptance.

## Current candidate: audio boundary, part 1 of 2
- Preserve native gameplay files. Convert only 16 exact existing gameplay SoX command literals to stable scalar IDs in a Web-only adapter.
- Fixed FIFO of 16 one-byte events; drop newest on overflow. Unknown/null commands are rejected; never run a shell. No allocation, sleeps or RNG calls.
- Export take/clear for the next browser consumer; include isolated C tests in make test.
- Sound remains disabled in web/main.c. This increment does NOT produce audible sound or implement SOUND controls.

## Remaining order
1. Validate this bridge in Codespace, then implement audio part 2: browser reproduction of existing effects, SOUND toggle, input-gesture activation, bounded voices and clear/stop on mute or focus loss. Audit and wire the original main.c sound sites in the Web entry without changing gameplay rules. No new music or firing effect.
2. Validate intro/transitions visually and audit bosses, collisions, configured progression, suspension and console/network behavior.
3. Persistence LAST: original saves/ranking, ranking celebration/name capture, reload/error tests. Ranking sound is deferred with that feature. SQLite and future resource/data evolution remain design considerations.
4. Final native/Web regression, documentation and PR review before merge.

## Constraints
MAX_NIVEIS remains 4; the incomplete fourth configuration is an explicit inherited error, not early victory. No new levels, accounts or assets. Native build/core remain unchanged. LOAD/ranking stay notices. User operates Codespace/browser and controls merges; agent runs short checks.
