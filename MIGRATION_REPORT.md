# Migration report — validated playable baseline

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Tested baseline: a510b1e46a10c820d70db67b10c4a9c09b65ce27.
Evidence: Codespace agent outputs relayed by the user and the user's manual browser confirmation. This assistant did not control that browser.

## Environment and commands
Linux Codespace; Emscripten 6.0.13; SDK Node v24.19.0. Python HTTP server on port 8000 with private forwarding. Browser/version not recorded.
From repository root:
```
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test
make -f web/Makefile test-js NODE="$EMSDK_NODE"
make -f web/Makefile
make -f web/Makefile serve
```
Reuse the server if already running. HTTP access is required; do not open via file://. Generated files stay under ignored web/build.

## Tested and passed
- User PC: original native gameplay, before repository creation.
- Codespace agent: native baseline built in a temporary git archive copy with make -B, exit 0; 26 system return-value warnings and one fread return-value warning.
- Codespace agent: framebuffer and runtime/input/timing/formatter C tests passed.
- Codespace agent: JavaScript syntax and mock-DOM tests passed using the SDK Node, including both Enter keys, focus reset, heap replacement and error handling.
- Codespace agent: playable Wasm build passed; forced build at 183017f passed; build at a510b1e updated served browser.js. Existing port 8000 returned HTTP 200.
- User: ASCII rendering and initial gameplay confirmed. After a510b1e, the user confirmed the requested manual check covering numpad Enter start, movement/fire and focus-loss reset. This is an initial playtest, not a complete level/progression or console audit.
- Assistant environment: runtime units passed with Wall/Wextra/Werror and ASan/UBSan; JavaScript mock tests passed. NumpadEnter regression failed before the fix and passed after it.

## Tested with failures, subsequently addressed
- Module.HEAPU8 access aborted in the browser. Commit 183017f exports HEAPU8; subsequent rendering was confirmed by the user.
- PATH node could not execute (Permission denied). test-js subsequently passed with NODE="$EMSDK_NODE"; no global permission changes were required.
- Numpad Enter did not start the game. Commit a510b1e maps NumpadEnter and Enter to the same C action; automated and user checks subsequently passed.

## Validated only by inspection / not yet tested
- Inspection: core game modules and native main/menu/Makefile remain unchanged; MAX_NIVEIS remains 4. Web orchestration has explicit coordinate bounds checks and zero-initialized state.
- Not yet comprehensively tested: bosses, damage/collision cases, all level transitions, long suspended-page behavior, full console/network audit and native rebuild after the initial header adaptation.
- Not implemented/tested: complete menus/remapping, original intro/transition animations, Q/N shortcuts, browser audio, save/ranking integration or IDBFS persistence.

## Known limits
Space fires once per press; movement uses fixed updates rather than OS repeat. Stage transitions currently wait for Enter. Boss blink follows rendering frames. SoX shell calls are disabled only in Web builds. Original blocking draw animations are not called by the Web entry.
The fourth level has no complete original configuration/AI. The Web entry reports missing configuration, without reducing MAX_NIVEIS or declaring victory.
Migration is incomplete. No full roadmap completion, complete gameplay equivalence or merge approval is claimed.
