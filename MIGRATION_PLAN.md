# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.

1. Map preview: build passed in Codespace with Emscripten 6.0.13; browser rendering unconfirmed.
2. Current increment: original input/update/draw/boss modules, C browser loop and keyboard bridge; awaiting Codespace compilation and manual playtest.
3. Pending: original menus/remapping/animations, Q/N shortcuts, audio decision, saves/ranking and IDBFS.
4. Validate after each increment through the user; merge only with recorded evidence.

Keep MAX_NIVEIS=4 and the native Makefile unchanged. No new levels.
Web-only terminal adapter retains existing drawing/input interfaces. Original main.c and menu.c are not linked into the Web build.
Fixed simulation intervals: 30/27/24 ms for configured levels, at most five catch-up updates; discard gaps over 250 ms. Rendering follows requestAnimationFrame.
Missing level configuration produces an explicit error screen, never an early victory. Complete migration remains pending.
