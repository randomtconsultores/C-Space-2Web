# Migration plan

Repository: randomtconsultores/C-Space-2Web. Branch: migration/wasm.
Baseline: 8490a9796f1bd2397e9f776f7c438f3a33e92dc0.

1. First increment: browser host, Emscripten build and bounded C ASCII framebuffer displaying the original level generator.
2. Next: integrate original gameplay, fixed timestep, keyboard and nonblocking UI states.
3. Then: adapt existing saves/ranking and validate IDBFS persistence.
4. Validate browser interactions through the user; record actual results before merging.

Keep the native Makefile and MAX_NIVEIS=4 unchanged. Do not add levels.
The fourth configuration is zero-initialized and boss rules cover only indices 0-2; this inherited issue is outside migration scope.
JavaScript presents C data; it does not implement game rules.
Use a pre element for the initial ASCII presentation. No framework or bundler.
The current browser entry is a static map preview, not the gameplay loop.
