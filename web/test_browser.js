"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const vm = require("node:vm");
const path = require("node:path");
const events = {}, docEvents = {}, calls = [], errors = [], captures = [];
const status = {textContent: ""};
const screen = {replaceChildren(text) { this.text = text; }};
const context = {
    TextDecoder, console: {error(error) { errors.push(error); }},
    window: {addEventListener(name, handler) { events[name] = handler; }},
    document: {
        hidden: false,
        getElementById(id) { return id === "status" ? status : screen; },
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
M._web_buffer = () => 1;
M._web_buffer_size = () => 6;
M._web_width = () => 2;
M._web_height = () => 2;
M.HEAPU8 = new Uint8Array([0, 65, 66, 10, 67, 68, 10]);
M.onRuntimeInitialized();
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
events.blur(); assert.equal(resets, 1);
const afterBlur = calls.length;
events.keyup(key("KeyQ")); assert.equal(calls.length, afterBlur);
context.document.hidden = true; docEvents.visibilitychange(); assert.equal(resets, 2);
M.spaceRender(); assert.equal(screen.text.data, "AB\nCD\n");
M.HEAPU8 = new Uint8Array([0, 69, 70, 10, 71, 72, 10]);
M.spaceRender(); assert.equal(screen.text.data, "EF\nGH\n");
M._web_buffer_size = () => 999;
M.spaceRender(); assert.equal(errors.length, 1); assert.equal(M.spaceRender, null);
assert.match(status.textContent, /Falha na apresentação/);
console.log("PASS: Escape/Q separation, remapped input, capture, focus reset and rendering (mock DOM)");
