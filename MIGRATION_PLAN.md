# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Native baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.
Latest user-validated browser baseline: e6cfff1ab0768dde831ab13ee999c7db20cba820.

## Implemented and checked through the user
- Wasm build, HTTP serving, bounded C ASCII framebuffer and initial gameplay.
- Original input/update/draw/boss modules behind the Web terminal adapter.
- Nonblocking main menu, credits and control capture/remapping.
- Both Enter variants, focus/input reset, Escape for submenus and Q through the original damage path.
- These are scoped playtests, not a complete progression/collision audit.

## Current candidate: binary introduction
- Port the original 80 frames at 40 ms each (nominal 3.2 seconds) into a timed C state.
- No usleep, blocking loop, worker, DOM game logic or new gameplay feature.
- Generate the level only after the intro, preserving the original order before map srand(seed).
- Do not update gameplay during the intro; clear queued/held C input before play.
- Preserve the last frame for its 40 ms; cap catch-up at five updates and discard gaps over 250 ms.
- Local isolated intro units passed; Codespace Wasm build and browser acceptance remain pending.

## Remaining order, agreed with the user
1. Validate the intro candidate, then restore original level-transition animations without blocking.
2. Decide and implement the adaptation of existing audio.
3. Validate bosses, collisions, configured level progression, suspended-page timing and console/network behavior.
4. Persistence LAST: original saves and ranking, including ranking celebration/name entry when connected to real data; verify reload and errors.
5. Final regression checks, documentation and PR review before merge.

## Constraints
- Keep MAX_NIVEIS=4 and the native build. No new levels or unrelated features.
- LOAD GAME and HIGH SCORES remain explicitly pending. No database or persistence added in this increment.
- Consider SQLite and future data/resource evolution during storage design, but do not add accounts, images/sounds or per-level history now. Storage/backend decisions remain deferred.
- The incomplete fourth level remains an inherited issue, not an early victory.
- User operates Codespace/browser and controls merges; the agent runs short checks; this assistant prepares code and documentation.
