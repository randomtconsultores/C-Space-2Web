"use strict";
const assert = require("node:assert/strict");
const fs = require("node:fs");
const vm = require("node:vm");
const path = require("node:path");
const events = {}, docEvents = {}, calls = [], errors = [];
const status = {textContent: ""};
const screen = {replaceChildren(text) { this.text = text; }};
const context = {
    TextDecoder, Set, console: {error(error) { errors.push(error); }},
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
M._web_input = (...args) => calls.push(args);
let resets = 0;
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
assert.deepEqual(calls, [[0, 1], [0, 0]]);
events.keydown(key("Space", {ctrlKey: true}));
assert.equal(calls.length, 2);
events.keydown(key("Space"));
assert.deepEqual(calls.at(-1), [4, 1]);
for (const code of ["Enter", "NumpadEnter"]) {
    const start = calls.length;
    events.keydown(key(code));
    events.keydown(key(code, {repeat: true}));
    events.keyup(key(code));
    assert.deepEqual(calls.slice(start), [[5, 1], [5, 0]], code);
}
for (const [code, action] of [["KeyQ", 6], ["ArrowUp", 7], ["ArrowDown", 8]]) {
    const start = calls.length;
    events.keydown(key(code)); events.keyup(key(code));
    assert.deepEqual(calls.slice(start), [[action, 1], [action, 0]], code);
}
const sharedStart = calls.length;
events.keydown(key("Enter")); events.keydown(key("NumpadEnter"));
events.keyup(key("Enter"));
assert.deepEqual(calls.slice(sharedStart), [[5, 1]]);
events.keyup(key("NumpadEnter"));
assert.deepEqual(calls.slice(sharedStart), [[5, 1], [5, 0]]);
events.blur(); assert.equal(resets, 1);
context.document.hidden = true;
docEvents.visibilitychange(); assert.equal(resets, 2);
M.spaceRender(); assert.equal(screen.text.data, "AB\nCD\n");
M.HEAPU8 = new Uint8Array([0, 69, 70, 10, 71, 72, 10]);
M.spaceRender(); assert.equal(screen.text.data, "EF\nGH\n");
M._web_buffer_size = () => 999;
M.spaceRender(); assert.equal(errors.length, 1);
assert.equal(M.spaceRender, null);
assert.match(status.textContent, /Falha na apresentação/);
console.log("PASS: browser menu input, shared Enter state, focus reset, heap replacement and error handling (mock DOM)");
