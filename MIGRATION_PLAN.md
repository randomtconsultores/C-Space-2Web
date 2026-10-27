# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Native baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.
Audio integration c5fe243 was validated in game by the user on 2026-10-27.

## Implemented
- Web framebuffer, native gameplay adapters, nonblocking menus, remapping,
  Enter variants, Escape navigation, Q damage, intro and timed transitions.
- Howler effects, generated menu music, SOUND initially ON, mute/focus handling.
- N-to-arena: C stage 5d9a403 plus this keyboard integration. Local C and JS
  tests passed; full Codespace build and N browser acceptance still pending.
- Initial gameplay and audio acceptance do not imply a complete level audit.

## Remaining order
1. Validate N in Codespace/browser, then audit configured levels, bosses,
   collisions, immunity, lives, score, transitions, restarts and focus.
2. Complete console/network and touch-control checks; confirm intro/transitions
   visually where not already explicitly accepted.
3. Persistence LAST: saves/LOAD, ranking, celebration/name capture, reload/errors.
   SQLite and future resource/data evolution remain design considerations only.
4. Final native/Web regression, documentation and PR review before merge.

## Constraints and roles
Keep MAX_NIVEIS=4. Incomplete fourth configuration remains an explicit inherited
error, not early victory. No new levels or accounts. Native core/build unchanged;
LOAD/ranking remain notices. Existing generated music and effects are retained.
Assistant writes code/reviews/commits after approval. Copilot executes validation
commands only. User validates in game and controls merges.
