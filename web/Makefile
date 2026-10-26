# Run from repository root. Validated baseline toolchain: Emscripten 6.0.13.
EMCC ?= emcc
CC ?= cc
NODE ?= node
PYTHON ?= python3
OUT := web/build
SOURCES := web/main.c web/framebuffer.c web/runtime.c web/terminal.c web/ui.c web/controls.c web/intro.c web/transition.c web/audio.c space.c levels.c boss.c draw.c
HEADERS := menu.h web/ui.h web/controls.h web/intro.h web/transition.h web/audio.h space.h levels.h boss.h draw.h web/framebuffer.h web/runtime.h web/terminal.h web/platform.h
TEST_COMMON := web/framebuffer.c web/runtime.c web/terminal.c web/controls.c web/test_game_globals.c
.PHONY: all check-toolchain test test-js serve
all: check-toolchain $(OUT)/space.js $(OUT)/space.wasm $(OUT)/index.html $(OUT)/browser.js
check-toolchain:
	@command -v $(EMCC) >/dev/null 2>&1 || { echo "emcc not found; activate Emscripten before building."; exit 1; }
$(OUT)/space.js $(OUT)/space.wasm &: $(SOURCES) $(HEADERS) web/Makefile
	mkdir -p $(OUT)
	$(EMCC) -std=gnu11 -Wall -Wextra -O0 -g -DSPACE_WEB -I. -include web/platform.h $(SOURCES) -lm -sASSERTIONS=1 -sENVIRONMENT=web -sFILESYSTEM=0 -sEXPORTED_RUNTIME_METHODS='["HEAPU8"]' -sEXPORTED_FUNCTIONS='["_main","_web_buffer","_web_buffer_size","_web_width","_web_height","_web_input","_web_reset","_web_control_key","_web_capture_active","_web_capture_key","_web_audio_take","_web_audio_clear"]' -o $(OUT)/space.js
$(OUT)/index.html: web/index.html
	mkdir -p $(OUT)
	cp $< $@
$(OUT)/browser.js: web/browser.js
	mkdir -p $(OUT)
	cp $< $@
test:
	@tmp=$$(mktemp -d) || exit 1; trap 'rm -rf "$$tmp"' EXIT HUP INT TERM; \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -Iweb web/framebuffer.c web/test_framebuffer.c -o "$$tmp/framebuffer" && "$$tmp/framebuffer" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/test_runtime.c -lm -o "$$tmp/runtime" && "$$tmp/runtime" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/ui.c web/test_ui.c -lm -o "$$tmp/ui" && "$$tmp/ui" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/ui.c web/test_controls.c -lm -o "$$tmp/controls" && "$$tmp/controls" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/intro.c web/test_intro.c -lm -o "$$tmp/intro" && "$$tmp/intro" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb web/framebuffer.c web/terminal.c web/transition.c web/test_transition.c -lm -o "$$tmp/transition" && "$$tmp/transition" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -Iweb web/audio.c web/test_audio.c -o "$$tmp/audio" && "$$tmp/audio"
test-js:
	$(NODE) --check web/browser.js
	$(NODE) web/test_browser.js
serve: all
	$(PYTHON) -m http.server 8000 --bind 0.0.0.0 --directory $(OUT)
