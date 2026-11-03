"use strict";
const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const vm = require("node:vm");
const {createClient, PersistenceError} = require("./persistence.js");
const A = "a".repeat(64), B = "b".repeat(64);
const base = {format_version: 1, level_index: 0, lives: 3, score: 0};
const reply = (data, status = 200, headers = {}) => new Response(JSON.stringify(data),
    {status, headers: {"Content-Type": "application/json", ...headers}});
const session = (token = A) => reply({csrf_token: token, expires_at: 9999999999}, 201);
const cp = (revision = 1, extra = {}) => ({...base, revision, updated_at: 123, ...extra});
const saved = (revision = 1, extra = {}) => reply({checkpoint: cp(revision, extra)});
const error = (status, code) => reply({error: {code}}, status);
const tick = () => new Promise(resolve => setImmediate(resolve));
function fixture(steps) {
    const calls = [], timers = new Map();
    let id = 0;
    const client = createClient({
        fetch: (url, init) => {
            calls.push({url, init});
            assert.ok(steps.length, "Unexpected request");
            const step = steps.shift();
            return typeof step === "function" ? step(url, init) : Promise.resolve(step);
        },
        setTimer: (fn, ms) => { assert.equal(ms, 10000); timers.set(++id, fn); return id; },
        clearTimer: key => timers.delete(key)
    });
    return {client, calls, timers, fire: () => { const pending = [...timers.values()]; timers.clear(); pending.forEach(fn => fn()); }};
}
const rejected = (promise, code, uncertain) => assert.rejects(promise, value => {
    assert.ok(value instanceof PersistenceError);
    assert.equal(value.code, code);
    if (uncertain !== undefined) assert.equal(value.uncertain, uncertain);
    return true;
});

test("classic script is inert and exports a browser namespace", () => {
    const sandbox = {};
    Object.defineProperty(sandbox, "fetch", {get() { throw new Error("Must not fetch at import"); }});
    vm.runInNewContext(fs.readFileSync(require.resolve("./persistence.js"), "utf8"), sandbox);
    assert.equal(typeof sandbox.SpacePersistence.createClient, "function");
    assert.ok(Object.isFrozen(sandbox.SpacePersistence));
});

test("missing save, snapshot copy, revision and transport security", async () => {
    const f = fixture([session(), error(404, "checkpoint_missing"), saved(), saved(2)]);
    assert.equal(await f.client.load(), null);
    const input = {...base};
    const pending = f.client.save(input);
    input.score = 900;
    const first = await pending;
    assert.ok(Object.isFrozen(first));
    assert.equal(first.revision, 1);
    assert.equal((await f.client.save(base)).revision, 2);
    assert.equal(f.client.getState().revision, 2);
    assert.equal(f.timers.size, 0);
    assert.ok(Object.isFrozen(f.client.getState()));
    for (const {url, init} of f.calls) {
        assert.ok(["/api/v1/session", "/api/v1/checkpoint"].includes(url));
        for (const [key, value] of Object.entries({credentials: "same-origin", mode: "same-origin", cache: "no-store", redirect: "error"}))
            assert.equal(init[key], value);
        assert.equal(init.headers.Cookie, undefined);
        assert.equal(init.headers.Origin, undefined);
        assert.ok(init.signal instanceof AbortSignal);
        if (url.endsWith("checkpoint")) assert.equal(init.headers["X-CSRF-Token"], A);
    }
    assert.deepEqual(JSON.parse(f.calls[2].init.body), {...base, expected_revision: 0});
    assert.equal(JSON.parse(f.calls[3].init.body).expected_revision, 1);
    assert.ok(!JSON.stringify(f.client.getState()).includes(A));
});

test("level 3 is preserved, not interpreted as victory", async () => {
    const f = fixture([session(), saved(5, {level_index: 3})]);
    const result = await f.client.load();
    assert.equal(result.level_index, 3);
    assert.equal(f.client.getState().revision, 5);
});

test("local validation and load prerequisite do not send requests", async () => {
    const f = fixture([]);
    await rejected(f.client.save(base), "load_required");
    for (const bad of [{...base, lives: true}, {...base, score: -1}, {...base, level_index: 4}, {...base, owner: "x"}])
        await rejected(f.client.save(bad), "invalid_snapshot");
    for (const options of [null, {baseURL: "https://other.test"}, {timeoutMs: 0}])
        assert.throws(() => createClient(options), PersistenceError);
    assert.equal(f.calls.length, 0);
});

test("disabled API is not mistaken for an empty checkpoint", async () => {
    const f = fixture([error(404, "not_found")]);
    await rejected(f.client.load(), "not_found");
    assert.equal(f.client.getState().revision, null);
    assert.equal(f.calls.length, 1);
});

test("HTTP failures never retry writes or leak server text", async () => {
    for (const [status, code] of [[401, "session_expired"], [403, "csrf_rejected"], [409, "revision_conflict"],
                                 [429, "session_rate_limited"], [503, "storage_unavailable"], [500, "private-secret"]]) {
        const f = fixture([session(), saved(2), error(status, code)]);
        await f.client.load();
        await rejected(f.client.save(base), status === 500 ? "http_error" : code, status >= 500);
        assert.equal(f.client.getState().revision, null);
        await rejected(f.client.save(base), "load_required");
        assert.equal(f.calls.length, 3);
        assert.equal(f.timers.size, 0);
    }
});

test("malformed, oversized and incompatible replies fail closed", async () => {
    const responses = [
        new Response("<html>private</html>", {headers: {"Content-Type": "text/html"}}),
        new Response("{", {headers: {"Content-Type": "application/json"}}),
        new Response(new Uint8Array([255]), {headers: {"Content-Type": "application/json"}}),
        new Response(" ".repeat(5000), {headers: {"Content-Type": "application/json"}}),
        reply({checkpoint: cp()}, 200, {"Content-Length": "9999"}),
        saved(0), saved(1, {lives: true}), reply({checkpoint: cp(), owner: "private"})
    ];
    for (const response of responses) {
        const f = fixture([session(), response]);
        await rejected(f.client.load(), "invalid_response");
        assert.equal(f.client.getState().revision, null);
        assert.equal(f.timers.size, 0);
    }
    const f = fixture([session(), saved(1, {format_version: 2})]);
    await rejected(f.client.load(), "unsupported_format");
});

test("cancel before dispatch sends nothing", async () => {
    const f = fixture([]);
    const pending = rejected(f.client.load(), "cancelled", false);
    f.client.cancel();
    await pending;
    assert.equal(f.calls.length, 0);
    assert.equal(f.timers.size, 0);
});

test("busy and late bootstrap cannot replace a newer session", async () => {
    let release;
    const delayed = new Promise(resolve => { release = resolve; });
    const f = fixture([() => delayed, session(B), saved(4), saved(5)]);
    const pending = rejected(f.client.load(), "cancelled", false);
    await tick();
    await rejected(f.client.load(), "busy");
    f.client.cancel();
    await pending;
    await f.client.load();
    release(session(A));
    await tick();
    await f.client.save(base);
    assert.equal(f.calls[3].init.headers["X-CSRF-Token"], B);
    assert.equal(f.client.getState().revision, 5);
    assert.equal(f.timers.size, 0);
});

for (const mode of ["cancelled", "timeout"]) test(`${mode} during PUT requires reconciliation and ignores late acknowledgement`, async () => {
    let release;
    const delayed = new Promise(resolve => { release = resolve; });
    const f = fixture([session(), saved(4), () => delayed, session(B), saved(6)]);
    await f.client.load();
    const pending = rejected(f.client.save(base), mode, true);
    await tick();
    if (mode === "timeout") f.fire(); else f.client.cancel();
    await pending;
    assert.equal(f.client.getState().busy, false);
    assert.equal(f.client.getState().revision, null);
    assert.equal(f.calls.length, 3);
    await rejected(f.client.save(base), "load_required");
    await f.client.load();
    release(saved(5));
    await tick();
    assert.equal(f.client.getState().revision, 6);
    assert.equal(f.timers.size, 0);
});

test("unknown write outcome and wrong acknowledgement cannot advance revision", async () => {
    for (const outcome of [() => { throw new Error("private-secret"); }, saved(9), saved(5, {score: 999})]) {
        const f = fixture([session(), saved(4), outcome]);
        await f.client.load();
        await assert.rejects(f.client.save(base), e => {
            assert.ok(e instanceof PersistenceError);
            assert.ok(e.uncertain);
            assert.ok(!e.message.includes("private-secret"));
            return true;
        });
        assert.equal(f.client.getState().revision, null);
        assert.equal(f.calls.length, 3);
        assert.equal(f.timers.size, 0);
    }
});

test("timeout also cancels a stalled response body", async () => {
    let cancelled = false;
    const body = new ReadableStream({
        start(controller) { controller.enqueue(new TextEncoder().encode('{"checkpoint":')); },
        cancel() { cancelled = true; }
    });
    const f = fixture([session(), new Response(body, {headers: {"Content-Type": "application/json"}})]);
    const pending = rejected(f.client.load(), "timeout", false);
    await tick();
    f.fire();
    await pending;
    await tick();
    assert.ok(cancelled);
    assert.equal(f.client.getState().busy, false);
    assert.equal(f.timers.size, 0);
});
