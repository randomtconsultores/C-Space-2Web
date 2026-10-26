# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Native baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.
Playable baseline validated by the user: a510b1e46a10c820d70db67b10c4a9c09b65ce27.

## Completed increments
- Browser host, bounded 40x15 C ASCII framebuffer and Emscripten build.
- Original input/update/draw/boss modules connected through a Web-only terminal adapter.
- Persistent C state, nonblocking main loop, fixed updates and independent rendering.
- Keyboard press/release, focus reset and both Enter variants.
- Codespace builds/tests and initial browser playtest confirmed through the user; see MIGRATION_REPORT.md for scope.

## Next increments
1. Restore the original main menu using nonblocking C UI states. Preserve the current playable path.
2. Restore controls/remapping, credits and remaining original UI/animations. Resolve Q/N shortcut behavior explicitly.
3. Integrate existing saves/ranking, load validation and serialized IDBFS synchronization; test persistence after reload.
4. Decide and document browser audio handling; currently disabled.
5. Validate bosses, collisions, all configured level transitions, suspended-page timing and extended-session console behavior. Finish documentation and review before merge.

## Constraints and decisions
- Keep MAX_NIVEIS=4 and the native Makefile unchanged. Do not add levels or unrelated features.
- Original main.c/menu.c are preserved but not linked into the current Web build. JavaScript handles presentation/input, not game rules.
- Fixed simulation intervals are 30/27/24 ms for configured levels, capped at five catch-up updates; gaps over 250 ms are discarded. Rendering follows requestAnimationFrame.
- Existing fourth-level configuration/AI incompleteness remains outside migration scope. An explicit error is not an early victory or migration completion.
- User operates Codespace/browser and controls merges. The agent runs short checks; this assistant prepares changes and maintains documentation.
