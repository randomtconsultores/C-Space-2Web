# Run from repository root. Validated baseline toolchain: Emscripten 6.0.13.
EMCC ?= emcc
CC ?= cc
NODE ?= node
PYTHON ?= python3
OUT := web/build
SOUND_IDS := 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
SOUND_FILES := $(addprefix $(OUT)/sounds/,$(addsuffix .wav,$(SOUND_IDS)))
SOUND_MANIFEST := $(OUT)/sounds/manifest.json
MENU_MUSIC := $(OUT)/sounds/menu.wav
SOURCES := web/main.c web/framebuffer.c web/runtime.c web/terminal.c web/ui.c web/controls.c web/intro.c web/transition.c web/audio.c web/checkpoint.c web/persist_bridge.c space.c levels.c boss.c draw.c
HEADERS := menu.h web/ui.h web/controls.h web/intro.h web/transition.h web/audio.h space.h levels.h boss.h draw.h web/framebuffer.h web/runtime.h web/terminal.h web/platform.h web/checkpoint.h web/persist_bridge.h
TEST_COMMON := web/framebuffer.c web/runtime.c web/terminal.c web/controls.c web/test_game_globals.c web/audio.c
.PHONY: all check-toolchain test test-js test-sound-assets serve test-persist-bridge
all: check-toolchain $(OUT)/space.js $(OUT)/space.wasm $(OUT)/index.html $(OUT)/browser.js $(OUT)/persistence.js $(OUT)/persistence_bridge.js $(SOUND_MANIFEST) $(SOUND_FILES) $(MENU_MUSIC)
check-toolchain:
	@command -v $(EMCC) >/dev/null 2>&1 || { echo "emcc not found; activate Emscripten before building."; exit 1; }
$(OUT)/space.js $(OUT)/space.wasm &: $(SOURCES) $(HEADERS) web/Makefile
	mkdir -p $(OUT)
	$(EMCC) -std=gnu11 -Wall -Wextra -O0 -g -DSPACE_WEB -I. -include web/platform.h $(SOURCES) -lm -sASSERTIONS=1 -sENVIRONMENT=web -sFILESYSTEM=0 -sEXPORTED_RUNTIME_METHODS='["HEAPU8"]' -sEXPORTED_FUNCTIONS='["_main","_web_buffer","_web_buffer_size","_web_width","_web_height","_web_input","_web_reset","_web_control_key","_web_capture_active","_web_capture_key","_web_audio_take","_web_audio_clear","_web_sound_enabled","_web_set_sound","_web_menu_active","_web_persist_kind","_web_persist_ticket","_web_persist_field","_web_persist_complete","_web_persist_cancel","_web_persist_bridge_set_available","_web_persist_begin_score_list","_web_persist_result_ready","_web_persist_score_count","_web_persist_score_position","_web_persist_score_score","_web_persist_score_name_char","_web_persist_score_feed_entry","_web_persist_score_feed_name_char","_web_persist_begin_score","_web_persist_name_feed","_web_persist_name_length","_web_persist_name_char","_web_persist_name_clear","_web_persist_name_take_action","_web_persist_name_active","_web_persist_name_start","_web_persist_name_stop","_web_persist_submit_name_char","_web_persist_submit_score","_web_persist_submit_position"]' -o $(OUT)/space.js
$(OUT)/index.html: web/index.html
	mkdir -p $(OUT)
	cp $< $@
$(OUT)/browser.js: web/browser.js
	mkdir -p $(OUT)
	cp $< $@
$(OUT)/persistence.js: web/persistence.js
	mkdir -p $(OUT)
	cp $< $@
$(OUT)/persistence_bridge.js: web/persistence_bridge.js
	mkdir -p $(OUT)
	cp $< $@
$(SOUND_MANIFEST) $(SOUND_FILES) $(MENU_MUSIC) &: web/make_sounds.py web/audio.h
	$(PYTHON) web/make_sounds.py --output $(OUT)/sounds
test: test-sound-assets
	@tmp=$$(mktemp -d) || exit 1; trap 'rm -rf "$$tmp"' EXIT HUP INT TERM; \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -Iweb web/framebuffer.c web/test_framebuffer.c -o "$$tmp/framebuffer" && "$$tmp/framebuffer" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/test_runtime.c -lm -o "$$tmp/runtime" && "$$tmp/runtime" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/persist_bridge.c web/checkpoint.c levels.c web/ui.c web/test_ui.c -lm -o "$$tmp/ui" && "$$tmp/ui" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/persist_bridge.c web/checkpoint.c levels.c web/ui.c web/test_controls.c -lm -o "$$tmp/controls" && "$$tmp/controls" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb $(TEST_COMMON) web/intro.c web/test_intro.c -lm -o "$$tmp/intro" && "$$tmp/intro" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -I. -Iweb web/framebuffer.c web/terminal.c web/transition.c web/test_transition.c -lm -o "$$tmp/transition" && "$$tmp/transition" && \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -Iweb web/audio.c web/test_audio.c -o "$$tmp/audio" && "$$tmp/audio"
test-sound-assets:
	$(PYTHON) -B web/test_sound_assets.py
test-js:
	$(NODE) --check web/browser.js
	$(NODE) web/test_browser.js
	$(NODE) web/test_n_shortcut.js
	$(NODE) --check web/persistence_bridge.js
	$(NODE) --check web/test_persistence_bridge.js
	$(NODE) --test web/test_persistence_bridge.js
	$(NODE) --check web/test_bridge_wiring.js
	$(NODE) web/test_bridge_wiring.js
test-persist-bridge:
	@tmp=$$(mktemp -d) || exit 1; trap 'rm -rf "$$tmp"' EXIT HUP INT TERM; \
	$(CC) -std=c11 -Wall -Wextra -Werror -O2 -DSPACE_WEB -DWEB_PERSIST_TEST -I. -Iweb -fsanitize=address,undefined web/persist_bridge.c web/checkpoint.c levels.c web/test_persist_bridge.c -lm -o "$$tmp/bridge" && "$$tmp/bridge"
serve: all
	$(PYTHON) -m http.server 8000 --bind 0.0.0.0 --directory $(OUT)
