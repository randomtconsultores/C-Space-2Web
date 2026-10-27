"use strict";
var MAX_SIMULTANEOUS_SOUNDS = 8;
var MAX_AUDIO_EVENT_ID = 16;
var statusNode = document.getElementById("status");
var audioStatusNode = document.getElementById("audio-status");
function spaceLoadError() {
    statusNode.textContent = "Falha ao carregar space.js. Confirma o build e os pedidos HTTP.";
}
var Module = {
    onAbort: function (reason) {
        statusNode.textContent = "Falha na inicialização Wasm: " + String(reason);
    },
    onRuntimeInitialized: function () {
        var decoder = new TextDecoder("utf-8", {fatal: true});
        var screen = document.getElementById("screen");
        var text = document.createTextNode("");
        screen.replaceChildren(text);
        var previous = "";
        var soundPlayers = new Map();
        var menuMusicPlayer = null;
        var menuMusicId = null;
        var activeSounds = new Map();
        var audioEnabled = false;
        var howlerWarningShown = false;
        var pageFocused = typeof document.hasFocus !== "function" || document.hasFocus();
        var audioContextReady = typeof Howler !== "undefined" && Howler.ctx && Howler.ctx.state === "running";
        var audioEventsReceived = 0;
        var audioPlayCalls = 0;
        var audioEffectPlayCalls = 0;
        var audioLoads = 0;
        var audioFailures = 0;
        var lastAudioEvent = 0;
        function pageIsActive() {
            return pageFocused && !document.hidden;
        }
        function updateAudioStatus(message) {
            if (!audioStatusNode) return;
            var enabled = typeof Module._web_sound_enabled === "function" &&
                Module._web_sound_enabled() !== 0;
            var contextState = typeof Howler !== "undefined" && Howler.ctx ? Howler.ctx.state : "unavailable";
            var globalVolume = typeof Howler !== "undefined" && typeof Howler.volume === "function" ? Howler.volume() : "n/a";
            var muted = typeof Howler !== "undefined" && Howler._muted ? "yes" : "no";
            audioStatusNode.textContent = "Audio " + (enabled ? "ON" : "OFF") +
                " | C events: " + audioEventsReceived +
                " | last ID: " + lastAudioEvent +
                " | play calls: " + audioPlayCalls +
                " | effect play calls: " + audioEffectPlayCalls +
                " | WAV loaded: " + audioLoads + "/" + (MAX_AUDIO_EVENT_ID + 1) +
                " | errors: " + audioFailures +
                " | context: " + contextState +
                " | volume: " + globalVolume +
                " | muted: " + muted + (message ? " | " + message : "");
        }
        function resumeAudioFromGesture() {
            if (!pageIsActive() || typeof Howler === "undefined" || !Howler.ctx ||
                typeof Howler.ctx.resume !== "function") return;
            try {
                var resume = Howler.ctx.resume();
                updateAudioStatus("unlock requested");
                if (resume && typeof resume.then === "function") {
                    resume.then(function () {
                        audioContextReady = true;
                        updateAudioStatus("audio context resumed");
                        syncMenuMusic();
                    })
                        .catch(function (error) {
                            ++audioFailures;
                            updateAudioStatus("AudioContext resume error: " + String(error));
                        });
                } else if (Howler.ctx.state === "running") {
                    audioContextReady = true;
                    syncMenuMusic();
                }
            } catch (error) {
                console.error(error);
            }
        }
        function createSoundPlayer(eventId) {
            let activePrefix = eventId + ":";
            return new Howl({
                src: ["sounds/" + eventId + ".wav"],
                volume: 0.6,
                preload: true,
                onload: function () { ++audioLoads; updateAudioStatus("WAV " + eventId + " loaded"); },
                onloaderror: function (id, error) {
                    ++audioFailures;
                    updateAudioStatus("WAV " + eventId + " load error: " + String(error));
                },
                onend: function (id) { activeSounds.delete(activePrefix + id); },
                onstop: function (id) { activeSounds.delete(activePrefix + id); },
                onplay: function (id) { updateAudioStatus("playing WAV " + eventId + " (#" + id + ")"); },
                onplayerror: function (id, error) {
                    activeSounds.delete(activePrefix + id);
                    ++audioFailures;
                    updateAudioStatus("play error WAV " + eventId + ": " + String(error));
                }
            });
        }
        if (typeof Howl === "function") {
            for (var soundId = 1; soundId <= MAX_AUDIO_EVENT_ID; ++soundId)
                soundPlayers.set(soundId, createSoundPlayer(soundId));
            menuMusicPlayer = new Howl({
                src: ["sounds/menu.wav"],
                volume: 0.18,
                loop: true,
                preload: true,
                onload: function () { ++audioLoads; updateAudioStatus("menu WAV loaded"); },
                onloaderror: function (id, error) {
                    ++audioFailures;
                    updateAudioStatus("menu WAV load error: " + String(error));
                },
                onplayerror: function (id, error) {
                    menuMusicId = null;
                    ++audioFailures;
                    updateAudioStatus("menu music play error: " + String(error));
                }
            });
        }
        function voiceCount() {
            return activeSounds.size + (menuMusicId === null ? 0 : 1);
        }
        function syncMenuMusic() {
            var enabled = typeof Module._web_sound_enabled === "function" &&
                Module._web_sound_enabled() !== 0;
            var inMenu = typeof Module._web_menu_active === "function" &&
                Module._web_menu_active() !== 0;
            if (pageIsActive() && audioContextReady && enabled && inMenu && menuMusicPlayer && menuMusicId === null &&
                voiceCount() < MAX_SIMULTANEOUS_SOUNDS) {
                menuMusicId = menuMusicPlayer.play();
                if (menuMusicId !== null && menuMusicId !== undefined) ++audioPlayCalls;
                updateAudioStatus("menu music requested");
            } else if ((!pageIsActive() || !audioContextReady || !enabled || !inMenu) && menuMusicId !== null) {
                menuMusicPlayer.stop(menuMusicId);
                menuMusicId = null;
            }
        }
        function stopAudio() {
            var playing = Array.from(activeSounds.values());
            activeSounds.clear();
            playing.forEach(function (sound) {
                try { sound.player.stop(sound.id); }
                catch (error) { console.error(error); }
            });
            if (menuMusicId !== null && menuMusicPlayer) {
                try { menuMusicPlayer.stop(menuMusicId); }
                catch (error) { console.error(error); }
                menuMusicId = null;
            }
            if (typeof Module._web_audio_clear === "function") Module._web_audio_clear();
        }
        function playSound(eventId, fromEventQueue) {
            var player = soundPlayers.get(eventId);
            if (!pageIsActive() || !audioContextReady || !Module._web_sound_enabled() ||
                !player || voiceCount() >= MAX_SIMULTANEOUS_SOUNDS) return;
            try {
                var playbackId = player.play();
                if (playbackId !== null && playbackId !== undefined) {
                    ++audioPlayCalls;
                    if (fromEventQueue) ++audioEffectPlayCalls;
                    activeSounds.set(eventId + ":" + playbackId, {player: player, id: playbackId});
                }
            } catch (error) {
                ++audioFailures;
                updateAudioStatus("play exception WAV " + eventId + ": " + String(error));
            }
        }
        function drainAudio() {
            updateSoundButton();
            syncMenuMusic();
            if (!pageIsActive()) {
                stopAudio();
                audioEnabled = false;
                updateAudioStatus("playback paused: page inactive");
                return;
            }
            var enabled = typeof Module._web_sound_enabled === "function" &&
                Module._web_sound_enabled() !== 0;
            if (!enabled) {
                if (audioEnabled || activeSounds.size) stopAudio();
                else if (typeof Module._web_audio_clear === "function") Module._web_audio_clear();
                audioEnabled = false;
                return;
            }
            audioEnabled = true;
            if (!audioContextReady) {
                updateAudioStatus("waiting for audio unlock");
                return;
            }
            if (typeof Module._web_audio_take !== "function") return;
            var eventId = Module._web_audio_take();
            while (eventId > 0) {
                ++audioEventsReceived;
                lastAudioEvent = eventId;
                if (eventId <= MAX_AUDIO_EVENT_ID && typeof Howl === "function" &&
                    voiceCount() < MAX_SIMULTANEOUS_SOUNDS) {
                    playSound(eventId, true);
                } else if (typeof Howl !== "function" && !howlerWarningShown) {
                    statusNode.textContent = "Howler.js indisponível; áudio desativado.";
                    howlerWarningShown = true;
                }
                eventId = Module._web_audio_take();
            }
            updateAudioStatus();
        }
        Module.spaceRender = function () {
            try {
                var pointer = Module._web_buffer();
                var size = Module._web_buffer_size();
                var expected = (Module._web_width() + 1) * Module._web_height();
                var heap = Module.HEAPU8;
                if (size !== expected || pointer < 0 || pointer > heap.length - size)
                    throw new Error("Framebuffer inválido");
                var next = decoder.decode(heap.subarray(pointer, pointer + size));
                if (next !== previous) { text.data = next; previous = next; }
            } catch (error) {
                statusNode.textContent = "Falha na apresentação: " + String(error);
                console.error(error);
                Module.spaceRender = null;
            }
            drainAudio();
        };
        var special = {Enter: 5, NumpadEnter: 5, Escape: 6, ArrowUp: 7, ArrowDown: 8};
        var pressed = new Map();
        var heldCounts = new Uint8Array(10);
        var touchControls = document.getElementById("touch-controls");
        var soundButton = document.getElementById("sound-toggle");
        var testSoundButton = document.getElementById("audio-test");
        var touchPresses = new Map();
        window.addEventListener("keydown", resumeAudioFromGesture, true);
        window.addEventListener("pointerdown", resumeAudioFromGesture, true);
        window.addEventListener("touchend", resumeAudioFromGesture, true);
        function updateSoundButton() {
            if (!soundButton || typeof Module._web_sound_enabled !== "function") return;
            var enabled = Module._web_sound_enabled() !== 0;
            soundButton.textContent = enabled ? "SOUND ON" : "SOUND OFF";
            soundButton.setAttribute("aria-pressed", enabled ? "true" : "false");
        }
        function pressActions(code, mapped) {
            if (pressed.has(code)) return;
            pressed.set(code, mapped);
            mapped.forEach(function (action) {
                if (heldCounts[action]++ === 0) Module._web_input(action, 1);
            });
        }
        function releaseActions(code) {
            if (!pressed.has(code)) return;
            var mapped = pressed.get(code);
            pressed.delete(code);
            mapped.forEach(function (action) {
                if (--heldCounts[action] === 0) Module._web_input(action, 0);
            });
        }
        if (soundButton) {
            soundButton.addEventListener("click", function (event) {
                event.preventDefault();
                resumeAudioFromGesture();
                if (typeof Module._web_set_sound === "function") {
                    var enabled = Module._web_sound_enabled() === 0;
                    Module._web_set_sound(enabled ? 1 : 0);
                    if (!enabled) {
                        stopAudio();
                        audioEnabled = false;
                    } else {
                        syncMenuMusic();
                    }
                    updateSoundButton();
                    updateAudioStatus();
                }
            });
        }
        if (testSoundButton) {
            testSoundButton.addEventListener("click", function (event) {
                event.preventDefault();
                resumeAudioFromGesture();
                if (!Module._web_sound_enabled()) Module._web_set_sound(1);
                audioEnabled = true;
                syncMenuMusic();
                updateSoundButton();
                playSound(13, false);
                updateAudioStatus("direct WAV 13 test");
            });
        }
        if (touchControls) {
            touchControls.addEventListener("pointerdown", function (event) {
                var button = event.target.closest && event.target.closest("button[data-action]");
                if (!button) return;
                event.preventDefault();
                var action = Number(button.getAttribute("data-action"));
                if (!Number.isInteger(action) || action < 0 || action > 9) return;
                var code = "Touch:" + event.pointerId;
                pressActions(code, [action]);
                touchPresses.set(event.pointerId, {code: code, started: Date.now()});
            });
            function releaseTouch(event) {
                var touch = touchPresses.get(event.pointerId);
                if (!touch) return;
                touchPresses.delete(event.pointerId);
                var delay = Math.max(0, 120 - (Date.now() - touch.started));
                setTimeout(function () { releaseActions(touch.code); }, delay);
            }
            touchControls.addEventListener("pointerup", releaseTouch);
            touchControls.addEventListener("pointercancel", releaseTouch);
        }
        updateSoundButton();
        updateAudioStatus("waiting for game event");
        function reset() {
            pressed.clear();
            heldCounts.fill(0);
            touchPresses.clear();
            Module._web_reset();
            stopAudio();
            audioEnabled = false;
        }
        function character(event) {
            var key = -1;
            if (typeof event.key === "string" && event.key.length === 1)
                key = event.key.charCodeAt(0);
            else if (/^Key[A-Z]$/.test(event.code)) key = event.code.charCodeAt(3);
            else if (/^Digit[0-9]$/.test(event.code)) key = event.code.charCodeAt(5);
            else if (event.code === "Space") key = 32;
            return key >= 65 && key <= 90 ? key + 32 : key;
        }
        function actions(event) {
            if (Object.prototype.hasOwnProperty.call(special, event.code))
                return [special[event.code]];
            var key = character(event), result = [];
            if (key === 113) result.push(9);
            /* Keep original W/S menu navigation independent of game bindings. */
            if (key === 119) result.push(7);
            if (key === 115) result.push(8);
            for (var i = 0; i < 5; ++i)
                if (key === Module._web_control_key(i)) result.push(i);
            return result;
        }
        window.addEventListener("keydown", function (event) {
            if (event.ctrlKey || event.metaKey || event.altKey) return;
            if (Module._web_capture_active()) {
                event.preventDefault();
                if (!event.repeat) Module._web_capture_key(event.key === "Escape" || event.code === "Escape" ? 27 : character(event));
                return;
            }
            var mapped = actions(event);
            if (!mapped.length) return;
            event.preventDefault();
            if (event.repeat || pressed.has(event.code)) return;
            pressActions(event.code, mapped);
        });
        window.addEventListener("keyup", function (event) {
            if (!pressed.has(event.code)) return;
            event.preventDefault();
            releaseActions(event.code);
        });
        window.addEventListener("blur", function () {
            pageFocused = false;
            reset();
        });
        function handlePageFocus() {
            pageFocused = typeof document.hasFocus !== "function" || document.hasFocus();
            if (pageIsActive() && audioContextReady) resumeAudioFromGesture();
        }
        window.addEventListener("focus", handlePageFocus);
        document.addEventListener("visibilitychange", function () {
            if (document.hidden) reset();
            else handlePageFocus();
        });
        statusNode.textContent = "Menu: W/S ou setas, Enter seleciona, Esc recua/cancela. Som opcional em Controls.";
    }
};
