# Run from the repository root: make -f web/Makefile
EMCC ?= emcc
CC ?= cc
PYTHON ?= python3
OUT := web/build
SOURCES := web/main.c web/framebuffer.c levels.c
HEADERS := space.h levels.h web/framebuffer.h
.PHONY: all check-toolchain test serve
all: check-toolchain $(OUT)/space.js $(OUT)/space.wasm $(OUT)/index.html $(OUT)/browser.js
check-toolchain:
	@command -v $(EMCC) >/dev/null 2>&1 || { echo "emcc not found; activate Emscripten before building."; exit 1; }
$(OUT)/space.js $(OUT)/space.wasm &: $(SOURCES) $(HEADERS) web/Makefile
	mkdir -p $(OUT)
	$(EMCC) -std=c11 -Wall -Wextra -O0 -g -DSPACE_WEB -I. $(SOURCES) -lm -sASSERTIONS=1 -sENVIRONMENT=web -sFILESYSTEM=0 -sEXPORTED_FUNCTIONS='["_main","_web_buffer","_web_buffer_size","_web_width","_web_height"]' -o $(OUT)/space.js
$(OUT)/index.html: web/index.html
	mkdir -p $(OUT)
	cp $< $@
$(OUT)/browser.js: web/browser.js
	mkdir -p $(OUT)
	cp $< $@
test:
	@tmp=$$(mktemp -d) || exit 1; trap 'rm -rf "$$tmp"' EXIT HUP INT TERM; \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -Iweb web/framebuffer.c web/test_framebuffer.c -o "$$tmp/test" && "$$tmp/test"
serve: all
	$(PYTHON) -m http.server 8000 --bind 0.0.0.0 --directory $(OUT)
