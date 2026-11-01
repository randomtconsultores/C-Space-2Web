"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");

const BROWSER_JS = fs.readFileSync(path.join(__dirname, "browser.js"), "utf8");

function makeElement() {
    return {
        textContent: "",
        children: [],
        listeners: {},
        addEventListener(type, fn) { this.listeners[type] = fn; },
        setAttribute() {},
        replaceChildren(...nodes) { this.children = nodes; }
    };
}

function makeSandbox() {
    const elements = new Map();
    const logs = [];
    const textNodes = [];
    const document = {
        hidden: false,
        getElementById(id) {
            if (!elements.has(id)) elements.set(id, makeElement());
            return elements.get(id);
        },
        createTextNode(data) {
            const node = { data: String(data) };
            textNodes.push(node);
            return node;
        },
        addEventListener() {},
        hasFocus() { return true; }
    };
    const sandbox = {
        document,
        window: { addEventListener() {} },
        TextDecoder,
        Howl: function Howl() {},
        Howler: {
            ctx: { state: "running", resume() { return Promise.resolve(); } },
            volume() { return 1; },
            _muted: false
        },
        console: { error(...args) { logs.push(args); } },
        setTimeout
    };
    return { sandbox, elements, logs, textNodes };
}

function addCExports(Module) {
    Module._web_buffer = () => 1;
    Module._web_buffer_size = () => 6;
    Module._web_width = () => 2;
    Module._web_height = () => 2;
    Module._web_sound_enabled = () => 0;
    Module._web_set_sound = () => {};
    Module._web_menu_active = () => 0;
    Module._web_audio_take = () => 0;
    Module._web_audio_clear = () => {};
    Module._web_control_key = () => -1;
    Module._web_capture_active = () => 0;
    Module._web_capture_key = () => {};
    Module._web_input = () => {};
    Module._web_reset = () => {};
    Module.HEAPU8 = new Uint8Array([0, 65, 66, 10, 67, 68, 10]);
}

function load(bridge, options = {}) {
    const env = makeSandbox();
    const context = vm.createContext(env.sandbox);
    vm.runInContext(BROWSER_JS, context, { filename: "browser.js" });
    const Module = context.Module;
    assert.ok(Module && typeof Module.onRuntimeInitialized === "function");
    if (options.exports !== false) addCExports(Module);
    if (bridge !== undefined) context.SpacePersistenceBridge = bridge;
    return { context, Module, ...env };
}

function makeBridge(overrides = {}) {
    const calls = { init: [], poll: 0 };
    const bridge = {
        init(m) { calls.init.push(m); },
        poll() { calls.poll++; },
        ...overrides
    };
    return { bridge, calls };
}

const EXPECTED_FRAME = "AB\nCD\n";

test("loading browser.js does not call init or poll", () => {
    const { bridge, calls } = makeBridge();
    load(bridge);
    assert.equal(calls.init.length, 0);
    assert.equal(calls.poll, 0);
});

test("onRuntimeInitialized calls init exactly once with context.Module", () => {
    const { bridge, calls } = makeBridge();
    const { context, Module } = load(bridge);
    Module.onRuntimeInitialized();
    assert.equal(calls.init.length, 1);
    assert.equal(calls.init[0], context.Module);
    assert.equal(calls.poll, 0);
});

test("each spaceRender calls poll exactly once", () => {
    const { bridge, calls } = makeBridge();
    const { Module } = load(bridge);
    Module.onRuntimeInitialized();
    Module.spaceRender();
    assert.equal(calls.poll, 1);
    Module.spaceRender();
    assert.equal(calls.poll, 2);
});

test("missing bridge does not break initialization or rendering", () => {
    const { Module, textNodes } = load(undefined);
    assert.doesNotThrow(() => Module.onRuntimeInitialized());
    assert.equal(typeof Module.spaceRender, "function");
    assert.doesNotThrow(() => Module.spaceRender());
    assert.equal(textNodes[0].data, EXPECTED_FRAME);
});

test("init exception does not stop remaining initialization", () => {
    const { bridge } = makeBridge({ init() { throw new Error("init failed"); } });
    const { Module, elements, textNodes } = load(bridge);
    assert.doesNotThrow(() => Module.onRuntimeInitialized());
    assert.match(elements.get("status").textContent, /^Menu:/);
    assert.equal(typeof Module.spaceRender, "function");
    assert.doesNotThrow(() => Module.spaceRender());
    assert.equal(textNodes[0].data, EXPECTED_FRAME);
});

test("poll exception does not escape or disable render", () => {
    const { bridge } = makeBridge({ poll() { throw new Error("poll failed"); } });
    const { Module, elements } = load(bridge);
    Module.onRuntimeInitialized();
    assert.doesNotThrow(() => Module.spaceRender());
    assert.equal(typeof Module.spaceRender, "function");
    assert.doesNotThrow(() => Module.spaceRender());
    assert.doesNotMatch(elements.get("status").textContent, /Falha na apresenta/);
});

test("valid framebuffer is displayed even when poll throws", () => {
    let polls = 0;
    const { bridge } = makeBridge({ poll() { polls++; throw new Error("poll failed"); } });
    const { Module, textNodes } = load(bridge);
    Module.onRuntimeInitialized();
    Module.spaceRender();
    assert.equal(polls, 1);
    assert.equal(textNodes[0].data, EXPECTED_FRAME);
});

test("framebuffer failure still lets poll run and is reported separately", () => {
    const { bridge, calls } = makeBridge();
    const { Module, elements } = load(bridge);
    Module.onRuntimeInitialized();
    Module._web_buffer_size = () => 5;
    Module.spaceRender();
    assert.equal(calls.poll, 1);
    assert.match(elements.get("status").textContent, /Falha na apresenta/);
    assert.equal(Module.spaceRender, null);
});
