# Native N shortcut — C stage

The native N action moves player.x to 2 and the scroll offset to the current
level's boss arena. It does not complete the stage or defeat/reset the boss.
The Web entry now applies that action before process_input, avoiding a negative
sentinel being interpreted as a map position. Normal update_game and arena
activation still run; this does not grant immunity or stop combat.

Input index 10 is reserved for N; existing indices 0..9 remain unchanged.
web_skip_pressed consumes one press, including a press/release between ticks.
Holding/repeated keydown does not trigger another jump until release/repress.
Q wins if Q and N arrive together; a rejected N is discarded, not replayed later.
The warp tick does not process movement/fire; queued input is flushed. Held
movement can resume normally on the following tick. Existing shots are not cleared.
Native gameplay files, MAX_NIVEIS=4, sound defaults and persistence are unchanged.

This commit is the C stage only. The next commit must map N/n to index 10 in
browser.js, extend its held-count array and add JS regressions. Until then,
pressing N on the page does not yet invoke this C action.

Local isolated runtime tests passed with Wall/Wextra/Werror and ASan/UBSan:
N edges/repeat/reset/flush, Q priority, Q regression, timestep and formatting.
Controls/framebuffer/terminal sources matched repository blob hashes; space.h
and globals were local fixtures. main.c routing was inspected, not built with
Emscripten here. Full Codespace build and real N/browser validation remain pending.
Audio at c5fe243 was validated in game by the user; do not report it as pending.

After both stages, validation commands from the repository root:
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test test-js NODE="$EMSDK_NODE"
make -B -f web/Makefile
