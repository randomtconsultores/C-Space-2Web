# Run from repository root. Validated preview toolchain: Emscripten 6.0.13.
EMCC ?= emcc
CC ?= cc
NODE ?= node
PYTHON ?= python3
OUT := web/build
SOURCES := web/main.c web/framebuffer.c web/runtime.c web/terminal.c space.c levels.c boss.c draw.c
HEADERS := space.h levels.h boss.h draw.h web/framebuffer.h web/runtime.h web/terminal.h web/platform.h
.PHONY: all check-toolchain test test-js serve
all: check-toolchain $(OUT)/space.js $(OUT)/space.wasm $(OUT)/index.html $(OUT)/browser.js
check-toolchain:
	@command -v $(EMCC) >/dev/null 2>&1 || { echo "emcc not found; activate Emscripten before building."; exit 1; }
$(OUT)/space.js $(OUT)/space.wasm &: $(SOURCES) $(HEADERS) web/Makefile
	mkdir -p $(OUT)
	$(EMCC) -std=gnu11 -Wall -Wextra -O0 -g -DSPACE_WEB -I. -include web/platform.h $(SOURCES) -lm -sASSERTIONS=1 -sENVIRONMENT=web -sFILESYSTEM=0 -sEXPORTED_RUNTIME_METHODS='["HEAPU8"]' -sEXPORTED_FUNCTIONS='["_main","_web_buffer","_web_buffer_size","_web_width","_web_height","_web_input","_web_reset"]' -o $(OUT)/space.js
$(OUT)/index.html: web/index.html
	mkdir -p $(OUT)
	cp $< $@
$(OUT)/browser.js: web/browser.js
	mkdir -p $(OUT)
	cp $< $@
test:
	@tmp=$$(mktemp -d) || exit 1; trap 'rm -rf "$$tmp"' EXIT HUP INT TERM; \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -Iweb web/framebuffer.c web/test_framebuffer.c -o "$$tmp/framebuffer" && "$$tmp/framebuffer" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -Iweb web/framebuffer.c web/runtime.c web/terminal.c web/test_runtime.c -lm -o "$$tmp/runtime" && "$$tmp/runtime"
test-js:
	$(NODE) --check web/browser.js
	$(NODE) web/test_browser.js
serve: all
	$(PYTHON) -m http.server 8000 --bind 0.0.0.0 --directory $(OUT)
