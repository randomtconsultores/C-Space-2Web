"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const vm = require("node:vm");
const path = require("node:path");
const events = {}, calls = [], captures = [];
let capturing = false, resets = 0;
const status = {textContent: ""};
const screen = {replaceChildren() {}};
const context = {
    TextDecoder, console,
    window: {addEventListener(name, handler, capture) { if (!capture) events[name] = handler; }},
    document: {
        hidden: false,
        hasFocus() { return true; },
        getElementById(id) { return id === "status" ? status : id === "screen" ? screen : null; },
        createTextNode(data) { return {data}; },
        addEventListener() {}
    }
};
vm.createContext(context);
vm.runInContext(fs.readFileSync(path.join(__dirname, "browser.js"), "utf8"), context);
const M = context.Module;
M._web_control_key = i => [119, 115, 97, 100, 32][i];
M._web_capture_active = () => Number(capturing);
M._web_capture_key = key => captures.push(key);
M._web_input = (...args) => calls.push(args);
M._web_reset = () => ++resets;
M._web_sound_enabled = () => 0;
M._web_audio_clear = () => {};
M.onRuntimeInitialized();
function key(value, extra = {}) { return {code: "KeyN", key: value, preventDefault() {}, ...extra}; }
for (const value of ["n", "N"]) {
    const start = calls.length;
    events.keydown(key(value));
    events.keydown(key(value, {repeat: true}));
    events.keydown(key(value));
    events.keyup(key(value));
    assert.deepEqual(calls.slice(start), [[10, 1], [10, 0]]);
}
const beforeModifiers = calls.length;
for (const modifier of ["ctrlKey", "altKey", "metaKey"])
    events.keydown(key("n", {[modifier]: true}));
assert.equal(calls.length, beforeModifiers);
capturing = true;
events.keydown(key("N")); events.keyup(key("N"));
assert.deepEqual(captures, [110]);
assert.equal(calls.length, beforeModifiers);
capturing = false;
events.keydown(key("n"));
assert.deepEqual(calls.at(-1), [10, 1]);
events.blur();
assert.equal(resets, 1);
const afterBlur = calls.length;
events.keyup(key("n"));
assert.equal(calls.length, afterBlur);
events.keydown(key("n")); events.keyup(key("n"));
assert.deepEqual(calls.slice(afterBlur), [[10, 1], [10, 0]]);
const beforeQ = calls.length;
events.keydown({code: "KeyQ", key: "q", preventDefault() {}});
events.keyup({code: "KeyQ", key: "q", preventDefault() {}});
assert.deepEqual(calls.slice(beforeQ), [[9, 1], [9, 0]]);
console.log("PASS: N/n mapping, repeat filtering, modifiers, capture, focus reset and Q separation");
