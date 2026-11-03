# Native N shortcut

N/n sends input index 10 through the existing browser input path. Indices 0..9
are unchanged; Q remains 9. The held-count array covers all 11 inputs.
The existing capture path forwards N to the C binding validator, which keeps
N reserved. Modified OS shortcuts and repeated keydowns do not trigger a jump.

C implementation: 5d9a403. web_skip_pressed consumes one N edge, including a
press/release between simulation ticks. Holding N requires release/repress to
jump again. Q has priority when both arrive together; rejected N is discarded.
In PLAYING only, main.c sets player.x=2 and offset=level length-screen width,
then flushes queued input. It handles N before native map input can inspect a
negative sentinel. Movement/fire are not processed on that warp tick; existing
shots remain, and normal combat/update_game still runs. No boss is killed/reset,
no stage is advanced and no immunity is granted by the shortcut.

Runtime tests passed locally with Wall/Wextra/Werror and ASan/UBSan, with the
fixture limitations recorded in the first C-stage commit. The second stage's
Node tests passed: N/n edges, repeats, modifiers, capture, focus reset and Q
separation; existing input/render/audio regressions also passed with mock DOM.
Full Codespace rebuild and actual N-to-arena browser acceptance remain pending.
Audio at c5fe243 was validated in game by the user and is not pending.

From the repository root, after updating without discarding local changes:
source "$HOME/emsdk/emsdk_env.sh"
make -f web/Makefile test test-js NODE="$EMSDK_NODE"
make -B -f web/Makefile

Browser acceptance: N reaches the current arena, not the next stage; the boss
still requires defeat. Check N/n, hold/repress, Q, capture and focus. Confirm
remaining lives/score and level are not changed by the shortcut itself.
