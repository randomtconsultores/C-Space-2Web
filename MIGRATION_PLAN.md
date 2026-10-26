# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Native baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.
Last user-validated browser baseline: e6cfff1ab0768dde831ab13ee999c7db20cba820.

## Completed baseline
Wasm build, HTTP serving, ASCII framebuffer, initial gameplay, nonblocking menu, control remapping, both Enter keys, Escape navigation and Q damage were checked through the user. This is not a full progression/collision audit.

## Animation increments awaiting acceptance
- Intro published at 8eb25067b7e86233ec1d909cf4752dd9764a0447: 80 timed frames; no Codespace/browser result supplied yet.
- Current transition candidate: port draw_transition from draw.c to a timed C state; remove the temporary Enter-to-advance gate.
- Preserve original sequence: four 150 ms explosion steps, 2000 ms blank pause, first-level upgrade flashes (3 x 800/200 ms), completion flashes (2 x 700/300 ms).
- Nominal durations: 7600 ms after level 1; 4600 ms after other non-final levels. The original final level skips the transition.
- One phase change per callback; every phase receives its full hold. Discard negative or over-250 ms deltas. No sleeps, threads, workers or new JS game rules.
- Stop gameplay ticks during animations; advance once and clear input before resuming. Preserve lives/score; do not replay the intro between levels.

## Remaining order
1. Codespace build and browser validation of intro and transitions.
2. Adapt existing audio.
3. Audit bosses, collisions, configured level progression, suspension and console/network behavior.
4. Persistence LAST: existing saves/ranking and reload/error tests. SQLite and future data/resource evolution remain design considerations, not new features now.
5. Final regression, documentation and PR review before merge.

## Constraints
MAX_NIVEIS stays 4; no new levels, accounts or assets. Native gameplay modules/build remain unchanged. The incomplete fourth configuration produces an explicit error, not an early victory. LOAD GAME/HIGH SCORES remain notices. User operates Codespace/browser and controls merges; the agent runs short checks.
