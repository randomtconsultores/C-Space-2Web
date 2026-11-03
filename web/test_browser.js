"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const vm = require("node:vm");
const path = require("node:path");
const events = {}, captureEvents = {}, docEvents = {}, calls = [], errors = [], captures = [];
const audioQueue = [], howlInstances = [], playedSounds = [], stoppedSounds = [];
const status = {textContent: ""};
const audioStatus = {textContent: ""};
const screen = {replaceChildren(text) { this.text = text; }};
const touchHandlers = {}, soundHandlers = {}, audioTestHandlers = {};
const touchControls = {addEventListener(name, handler) { touchHandlers[name] = handler; }};
const soundButton = {
    textContent: "SOUND OFF",
    addEventListener(name, handler) { soundHandlers[name] = handler; },
    setAttribute(name, value) { this[name] = value; }
};
const audioTestButton = {addEventListener(name, handler) { audioTestHandlers[name] = handler; }};
let soundEnabled = 1, menuActive = 1, pageFocused = true, audioClears = 0, audioResumeCalls = 0;
class FakeHowl {
    constructor(options) { this.options = options; this.src = options.src[0]; this.nextId = 0; howlInstances.push(this); }
    play() { const id = this.nextId++; playedSounds.push([this.src, id]); return id; }
    stop(id) { stoppedSounds.push([this.src, id]); if (this.options.onstop) this.options.onstop(id); }
}
const context = {
    TextDecoder, console: {error(error) { errors.push(error); }},
    Howl: FakeHowl,
    Howler: {ctx: {state: "suspended", resume() {
        ++audioResumeCalls;
        this.state = "running";
        return {then(callback) { callback(); return {catch() {}}; }};
    }}},
    window: {addEventListener(name, handler, options) {
        if (options === true || (options && options.capture)) captureEvents[name] = handler;
        else events[name] = handler;
    }},
    document: {
        hidden: false,
        hasFocus() { return pageFocused; },
        getElementById(id) {
            if (id === "status") return status;
            if (id === "audio-status") return audioStatus;
            if (id === "touch-controls") return touchControls;
            if (id === "sound-toggle") return soundButton;
            if (id === "audio-test") return audioTestButton;
            return screen;
        },
        createTextNode(data) { return {data}; },
        addEventListener(name, handler) { docEvents[name] = handler; }
    }
};
vm.createContext(context);
vm.runInContext(fs.readFileSync(path.join(__dirname, "browser.js"), "utf8"), context);
const M = context.Module;
const bindings = [119, 115, 97, 100, 32];
let capturing = false, resets = 0;
M._web_control_key = action => bindings[action];
M._web_capture_active = () => Number(capturing);
M._web_capture_key = key => captures.push(key);
M._web_input = (...args) => calls.push(args);
M._web_reset = () => ++resets;
M._web_sound_enabled = () => soundEnabled;
M._web_menu_active = () => menuActive;
M._web_set_sound = enabled => {
    soundEnabled = enabled;
    if (!enabled) { audioQueue.length = 0; ++audioClears; }
};
M._web_audio_take = () => audioQueue.length ? audioQueue.shift() : 0;
M._web_audio_clear = () => { audioQueue.length = 0; ++audioClears; };
M._web_buffer = () => 1;
M._web_buffer_size = () => 6;
M._web_width = () => 2;
M._web_height = () => 2;
M.HEAPU8 = new Uint8Array([0, 65, 66, 10, 67, 68, 10]);
M.onRuntimeInitialized();
assert.equal(howlInstances.length, 17);
assert.equal(howlInstances[0].options.preload, true);
assert.equal(howlInstances[16].options.preload, true);
assert.equal(howlInstances[16].src, "sounds/menu.wav");
captureEvents.keydown(key("Enter"));
assert.equal(audioResumeCalls, 1);
assert.match(audioStatus.textContent, /Audio ON/);
assert.deepEqual(playedSounds[0], ["sounds/menu.wav", 0]);
pageFocused = false;
events.blur();
const playsOnBlur = playedSounds.length;
for (let i = 0; i < 5; ++i) M.spaceRender();
assert.equal(playedSounds.length, playsOnBlur);
assert.ok(stoppedSounds.some(sound => sound[0] === "sounds/menu.wav"));
pageFocused = true;
events.focus();
assert.equal(playedSounds.length, playsOnBlur + 1);
function key(code, extra = {}) { return {code, preventDefault() {}, ...extra}; }
events.keydown(key("KeyW"));
events.keydown(key("KeyW", {repeat: true}));
events.keyup(key("KeyW"));
assert.deepEqual(calls, [[7, 1], [0, 1], [7, 0], [0, 0]]);
events.keydown(key("Space", {ctrlKey: true})); assert.equal(calls.length, 4);
events.keydown(key("Space")); assert.deepEqual(calls.at(-1), [4, 1]);
for (const [code, action] of [["Enter", 5], ["NumpadEnter", 5], ["Escape", 6], ["KeyQ", 9], ["ArrowUp", 7], ["ArrowDown", 8]]) {
    const start = calls.length;
    events.keydown(key(code)); events.keydown(key(code, {repeat: true})); events.keyup(key(code));
    assert.deepEqual(calls.slice(start), [[action, 1], [action, 0]], code);
}
const shared = calls.length;
events.keydown(key("Enter")); events.keydown(key("NumpadEnter")); events.keyup(key("Enter"));
assert.deepEqual(calls.slice(shared), [[5, 1]]);
events.keyup(key("NumpadEnter")); assert.deepEqual(calls.slice(shared), [[5, 1], [5, 0]]);
const beforeCapture = calls.length;
capturing = true;
events.keydown(key("KeyK", {key: "K"}));
events.keydown(key("KeyK", {key: "K", repeat: true}));
events.keydown(key("KeyQ", {key: "q"}));
events.keydown(key("Escape", {key: "Escape"}));
assert.deepEqual(captures, [107, 113, 27]); assert.equal(calls.length, beforeCapture);
capturing = false; bindings[0] = 107;
let start = calls.length;
events.keydown(key("KeyW", {key: "w"})); events.keyup(key("KeyW", {key: "w"}));
assert.deepEqual(calls.slice(start), [[7, 1], [7, 0]]);
start = calls.length;
events.keydown(key("KeyK", {key: "k"}));
bindings[0] = 106;
events.keyup(key("KeyK", {key: "k"}));
assert.deepEqual(calls.slice(start), [[0, 1], [0, 0]]);
events.keydown(key("KeyQ"));
pageFocused = false;
events.blur(); assert.equal(resets, 2);
const afterBlur = calls.length;
events.keyup(key("KeyQ")); assert.equal(calls.length, afterBlur);
context.document.hidden = true; docEvents.visibilitychange(); assert.equal(resets, 3);
const playsWhileHidden = playedSounds.length;
for (let i = 0; i < 5; ++i) M.spaceRender();
assert.equal(playedSounds.length, playsWhileHidden);
context.document.hidden = false;
pageFocused = true;
docEvents.visibilitychange();
context.setTimeout = callback => callback();
touchHandlers.pointerdown({
    target: {closest() { return {getAttribute() { return "9"; }}; }},
    pointerId: 41,
    preventDefault() {}
});
assert.deepEqual(calls.at(-1), [9, 1]);
touchHandlers.pointerup({pointerId: 41});
assert.deepEqual(calls.slice(-2), [[9, 1], [9, 0]]);
touchHandlers.pointerdown({
    target: {closest() { return {getAttribute() { return "5"; }}; }},
    pointerId: 42,
    preventDefault() {}
});
assert.deepEqual(calls.at(-1), [5, 1]);
touchHandlers.pointerup({pointerId: 42});
soundHandlers.click({preventDefault() {}});
assert.equal(soundEnabled, 0);
assert.equal(soundButton.textContent, "SOUND OFF");
soundHandlers.click({preventDefault() {}});
assert.equal(soundEnabled, 1);
assert.equal(soundButton.textContent, "SOUND ON");
assert.ok(stoppedSounds.some(sound => sound[0] === "sounds/menu.wav"));
audioTestHandlers.click({preventDefault() {}});
assert.equal(playedSounds.at(-1)[0], "sounds/13.wav");
menuActive = 0;
pageFocused = false;
events.blur();
playedSounds.length = 0;
stoppedSounds.length = 0;
pageFocused = true;
events.focus();
M.spaceRender(); assert.equal(screen.text.data, "AB\nCD\n");
M.HEAPU8 = new Uint8Array([0, 69, 70, 10, 71, 72, 10]);
M.spaceRender(); assert.equal(screen.text.data, "EF\nGH\n");
for (let i = 0; i < 12; ++i) audioQueue.push(i % 2 + 1);
M.spaceRender();
assert.equal(playedSounds.length, context.MAX_SIMULTANEOUS_SOUNDS);
assert.equal(audioQueue.length, 0);
assert.match(audioStatus.textContent, /C events: 12/);
assert.match(audioStatus.textContent, /effect play calls: 8/);
assert.equal(howlInstances[0].src, "sounds/1.wav");
soundEnabled = 0;
audioQueue.push(3);
M.spaceRender();
assert.equal(stoppedSounds.length, context.MAX_SIMULTANEOUS_SOUNDS);
assert.equal(audioQueue.length, 0);
assert.ok(audioClears > 0);
soundEnabled = 1;
audioQueue.push(4);
M.spaceRender();
assert.equal(playedSounds.length, context.MAX_SIMULTANEOUS_SOUNDS + 1);
events.blur();
assert.equal(stoppedSounds.length, context.MAX_SIMULTANEOUS_SOUNDS + 1);
assert.equal(audioQueue.length, 0);
const playsAfterFinalBlur = playedSounds.length;
for (let i = 0; i < 5; ++i) M.spaceRender();
assert.equal(playedSounds.length, playsAfterFinalBlur);
assert.equal(audioQueue.length, 0);
M._web_buffer_size = () => 999;
M.spaceRender(); assert.equal(errors.length, 1); assert.equal(M.spaceRender, null);
assert.match(status.textContent, /Falha na apresentação/);
console.log("PASS: Escape/Q, remapping, focus reset, audio queue, voice limit and stop (mock DOM)");
