// web/test_persistence_bridge.js
"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");

function freshBridge() {
    delete require.cache[require.resolve("./persistence_bridge.js")];
    return require("./persistence_bridge.js");
}

function createMockModule() {
    let kind = 0;
    let ticket = 0;
    const fields = { 0: 1, 1: 0, 2: 3, 3: 1000 };
    let completeCalls = [];

    return {
        _web_persist_kind: () => kind,
        _web_persist_ticket: () => ticket,
        _web_persist_field: (t, f) => (t === ticket && fields[f] !== undefined ? fields[f] : -1),
        _web_persist_complete: (t, o, fv, li, lv, sc) => {
            completeCalls.push({ ticket: t, outcome: o, fv, li, lv, sc });
        },
        _completeCalls: () => completeCalls,
        _setKind: (k) => { kind = k; },
        _setTicket: (t) => { ticket = t; },
        _setField: (f, v) => { fields[f] = v; },
        _resetCompleteCalls: () => { completeCalls = []; }
    };
}

function createMockClient(behavior = {}) {
    let loadResolver, saveResolver;
    let revision = behavior.initialRevision ?? null;
    let loadResult = behavior.loadResult ?? null;
    let saveResult = behavior.saveResult ?? { format_version: 1, level_index: 0, lives: 3, score: 1000, revision: 1, updated_at: 123 };
    const loads = [];
    const saves = [];
    const scores = [];
    let scoresResolver;

    const client = {
        load: () => {
            const p = new Promise((resolve, reject) => {
                loadResolver = { resolve, reject };
                loads.push({ resolve, reject });
            });
            return p;
        },
        save: (snapshot) => {
            const p = new Promise((resolve, reject) => {
                saveResolver = { resolve, reject, snapshot };
                saves.push({ resolve, reject, snapshot });
            });
            return p;
        },
        scores: () => {
            const p = new Promise((resolve, reject) => {
                scoresResolver = { resolve, reject };
                scores.push({ resolve, reject });
            });
            return p;
        },
        cancel: () => {
            if (loadResolver) loadResolver.reject(new Error("cancelled"));
            if (saveResolver) saveResolver.reject(new Error("cancelled"));
        },
        getState: () => ({ revision, busy: false }),
        _resolveLoad: (value) => { if (loadResolver) loadResolver.resolve(value); },
        _rejectLoad: (err) => { if (loadResolver) loadResolver.reject(err); },
        _resolveSave: (value) => { if (saveResolver) saveResolver.resolve(value); },
        _rejectSave: (err) => { if (saveResolver) saveResolver.reject(err); },
        _resolveScores: (v) => { if (scoresResolver) scoresResolver.resolve(v); },
        _rejectScores: (e) => { if (scoresResolver) scoresResolver.reject(e); },
        _setRevision: (r) => { revision = r; },
        _loads: () => loads,
        _saves: () => saves,
        _scores: () => scores,
        // Expor resultados para os testes
        loadResult,
        saveResult
    };
    return client;
}

async function tick() {
    await new Promise(r => setImmediate(r));
}

// ===== Testes =====

test("init fails gracefully without SpacePersistence", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    // Sem globalThis.SpacePersistence, init falha
    assert.equal(Bridge.init(Module), false);
    assert.doesNotThrow(() => Bridge.poll());
    assert.doesNotThrow(() => Bridge.cancel());
});

test("init succeeds with injected client", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    assert.equal(Bridge.init(Module, { client }), true);
    assert.equal(Bridge.init(Module, { client }), false); // duplo
});

test("init accepts undefined/null options", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    globalThis.SpacePersistence = { createClient: () => client };
    assert.equal(Bridge.init(Module, undefined), true);
    delete globalThis.SpacePersistence;
    assert.equal(Bridge.init(Module, null), false); // segundo init falha
});

test("poll before init does nothing", () => {
    const Bridge = freshBridge();
    assert.doesNotThrow(() => Bridge.poll());
});

test("LOAD ok", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ loadResult: { format_version: 1, level_index: 0, lives: 3, score: 1000, revision: 1, updated_at: 123 } });
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(42);
    Bridge.poll();
    await tick();
    client._resolveLoad(client.loadResult);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 1);
    assert.deepEqual(calls[0], { ticket: 42, outcome: 1, fv: 1, li: 0, lv: 3, sc: 1000 });
});

test("LOAD null -> outcome 0", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ loadResult: null });
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(1);
    Bridge.poll();
    await tick();
    client._resolveLoad(null);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls[0].outcome, 0);
});

test("LOAD not_found error -> -1", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(2);
    Bridge.poll();
    await tick();
    client._rejectLoad({ code: "not_found" });
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls[0].outcome, -1);
});

test("LOAD other errors map correctly", async () => {
    const errorCases = [
        ["invalid_response", -2],
        ["unsupported_format", -2],
        ["invalid_snapshot", -2],
        ["load_required", -1],
        ["revision_conflict", -1],
        ["session_expired", -1],
        ["csrf_rejected", -1],
        ["session_rate_limited", -1],
        ["storage_unavailable", -1],
        ["http_error", -1],
        ["network_error", -1],
        ["unknown_code", -1],
        ["constructor", -1] // prototype pollution attempt
    ];

    for (const [code, expected] of errorCases) {
        const Bridge = freshBridge();
        const Module = createMockModule();
        const client = createMockClient();
        Bridge.init(Module, { client });

        Module._setKind(1);
        Module._setTicket(1);
        Bridge.poll();
        await tick();
        client._rejectLoad({ code });
        await tick();

        const calls = Module._completeCalls();
        assert.equal(calls[0].outcome, expected, `code ${code} -> ${expected}`);
    }
});

test("SAVE ok with existing revision", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ initialRevision: 5, saveResult: { format_version: 1, level_index: 1, lives: 3, score: 5000, revision: 6, updated_at: 456 } });
    Bridge.init(Module, { client });

    Module._setKind(2);
    Module._setTicket(10);
    Module._setField(0, 1);
    Module._setField(1, 1);
    Module._setField(2, 3);
    Module._setField(3, 5000);
    Bridge.poll();
    await tick();
    client._resolveSave(client.saveResult);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls[0].outcome, 1);
    assert.equal(calls[0].fv, 1);
    assert.equal(calls[0].li, 1);
    assert.equal(calls[0].lv, 3);
    assert.equal(calls[0].sc, 5000);
    assert.equal(client._saves().length, 1);
});

test("SAVE does load first when revision is null", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({
        initialRevision: null,
        loadResult: { format_version: 1, level_index: 0, lives: 3, score: 0, revision: 1, updated_at: 100 },
        saveResult: { format_version: 1, level_index: 1, lives: 3, score: 5000, revision: 2, updated_at: 200 }
    });
    Bridge.init(Module, { client });

    Module._setKind(2);
    Module._setTicket(5);
    Module._setField(0, 1);
    Module._setField(1, 1);
    Module._setField(2, 3);
    Module._setField(3, 5000);
    Bridge.poll();
    await tick();

    // Primeiro resolve load
    client._resolveLoad(client.loadResult);
    await tick();
    // Depois resolve save
    client._resolveSave(client.saveResult);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 1);
    assert.equal(calls[0].outcome, 1);
    assert.equal(client._loads().length, 1);
    assert.equal(client._saves().length, 1);
});

test("SAVE field -1 -> outcome -2", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ initialRevision: 1 });
    Bridge.init(Module, { client });

    Module._setKind(2);
    Module._setTicket(7);
    Module._setField(0, 1);
    Module._setField(1, -1);
    Module._setField(2, 3);
    Module._setField(3, 1000);
    Bridge.poll();
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls[0].outcome, -2);
    assert.equal(calls[0].fv, 0);
    assert.equal(calls[0].li, 0);
    assert.equal(calls[0].lv, 0);
    assert.equal(calls[0].sc, 0);
});

test("C cancels mid LOAD: client.cancel called, complete NOT called", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(100);
    Bridge.poll();
    await tick();

    // C cancela
    Module._setKind(0);
    Module._setTicket(0);
    Bridge.poll();
    await tick();

    // Resolve o load antigo
    client._resolveLoad({ format_version: 1, level_index: 0, lives: 3, score: 1000, revision: 1, updated_at: 1 });
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 0);
});

test("C cancels mid SAVE: client.cancel called, complete NOT called", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ initialRevision: 1 });
    Bridge.init(Module, { client });

    Module._setKind(2);
    Module._setTicket(200);
    Module._setField(0, 1);
    Module._setField(1, 1);
    Module._setField(2, 3);
    Module._setField(3, 1000);
    Bridge.poll();
    await tick();

    Module._setKind(0);
    Module._setTicket(0);
    Bridge.poll();
    await tick();

    client._resolveSave({ format_version: 1, level_index: 1, lives: 3, score: 1000, revision: 2, updated_at: 2 });
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 0);
    assert.equal(client._saves().length, 1); // a escrita já ia em voo
});

test("C cancels during pre-load of SAVE: no save called", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ initialRevision: null });
    Bridge.init(Module, { client });

    Module._setKind(2);
    Module._setTicket(300);
    Module._setField(0, 1);
    Module._setField(1, 1);
    Module._setField(2, 3);
    Module._setField(3, 1000);
    Bridge.poll();
    await tick();

    Module._setKind(0);
    Module._setTicket(0);
    Bridge.poll();
    await tick();

    client._resolveLoad({ format_version: 1, level_index: 0, lives: 3, score: 0, revision: 1, updated_at: 1 });
    await tick();
    const calls = Module._completeCalls();
    assert.equal(calls.length, 0);
    assert.equal(client._saves().length, 0);
});

test("ticket changes mid-operation: old discarded, new started", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ loadResult: { format_version: 1, level_index: 0, lives: 3, score: 1000, revision: 1, updated_at: 1 } });
    Bridge.init(Module, { client });

    // LOAD ticket 1
    Module._setKind(1);
    Module._setTicket(1);
    Bridge.poll();
    await tick();

    // Muda para SAVE ticket 2
    Module._setKind(2);
    Module._setTicket(2);
    Module._setField(0, 1);
    Module._setField(1, 1);
    Module._setField(2, 3);
    Module._setField(3, 2000);
    Bridge.poll();
    await tick();

    client._resolveLoad(client.loadResult);
    await tick();
    client._resolveSave({ format_version: 1, level_index: 1, lives: 3, score: 2000, revision: 2, updated_at: 2 });
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 1);
    assert.equal(calls[0].ticket, 2);
    assert.equal(calls[0].outcome, 1);
});

test("late response ignored after ticket change", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ loadResult: { format_version: 1, level_index: 0, lives: 3, score: 1000, revision: 1, updated_at: 1 } });
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(10);
    Bridge.poll();
    await tick();

    Module._setKind(1);
    Module._setTicket(20);
    Bridge.poll();
    await tick();

    // Resolve o pedido antigo (ticket 10)
    client._loads()[0].resolve(client.loadResult);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 0); // resposta tardia descartada
});

test("repeated poll does not relaunch operation", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ loadResult: { format_version: 1, level_index: 0, lives: 3, score: 1000, revision: 1, updated_at: 1 } });
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(99);
    Bridge.poll();
    Bridge.poll();
    Bridge.poll();
    await tick();

    client._resolveLoad(client.loadResult);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 1);
});

test("cancelled -> -3 only when C still expects it", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(1);
    Bridge.poll();
    await tick();

    // C cancela ANTES de rejeitarmos
    Module._setKind(0);
    Module._setTicket(0);
    Bridge.poll();
    await tick();

    client._rejectLoad({ code: "cancelled" });
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 0); // descartado silenciosamente
});

test("save throws synchronously -> exactly one complete(-1) in 5 polls", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ initialRevision: 1 });
    // save lança de forma síncrona
    let saveCalls = 0;
    client.save = () => { saveCalls++; throw new Error("sync boom"); };
    Bridge.init(Module, { client });

    Module._setKind(2);
    Module._setTicket(50);
    Module._setField(0, 1);
    Module._setField(1, 1);
    Module._setField(2, 3);
    Module._setField(3, 1000);

    // 5 polls
    for (let i = 0; i < 5; i++) Bridge.poll();
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 1);
    assert.equal(calls[0].outcome, -1);
    assert.equal(saveCalls, 1);
});

test("complete throws -> no relaunch", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ loadResult: { format_version: 1, level_index: 0, lives: 3, score: 1000, revision: 1, updated_at: 1 } });
    // _web_persist_complete lança
    let completeCalls = 0;
    Module._web_persist_complete = () => { completeCalls++; throw new Error("complete boom"); };
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(1);
    Bridge.poll();
    await tick();
    client._resolveLoad(client.loadResult);
    await tick();

    // Polls adicionais não devem relançar
    Bridge.poll();
    Bridge.poll();
    await tick();

    assert.equal(completeCalls, 1); // apenas a primeira tentativa
    assert.equal(client._loads().length, 1);
});

test("complete throws on SAVE -> no relaunch", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient({ initialRevision: 1, saveResult: { format_version: 1, level_index: 1, lives: 3, score: 5000, revision: 2, updated_at: 2 } });
    let completeCalls = 0;
    Module._web_persist_complete = () => { completeCalls++; throw new Error("complete boom"); };
    Bridge.init(Module, { client });

    Module._setKind(2);
    Module._setTicket(1);
    Module._setField(0, 1);
    Module._setField(1, 1);
    Module._setField(2, 3);
    Module._setField(3, 5000);
    Bridge.poll();
    await tick();
    client._resolveSave(client.saveResult);
    await tick();

    Bridge.poll();
    Bridge.poll();
    await tick();

    assert.equal(completeCalls, 1);
    assert.equal(client._saves().length, 1);
});

test("init double returns false", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    assert.equal(Bridge.init(Module, { client }), true);
    assert.equal(Bridge.init(Module, { client }), false);
});

test("client exceptions don't escape poll", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = {
        load: () => { throw new Error("boom"); },
        cancel: () => { throw new Error("boom2"); },
        getState: () => { throw new Error("boom3"); }
    };
    Bridge.init(Module, { client });
    assert.doesNotThrow(() => Bridge.poll());
    assert.doesNotThrow(() => Bridge.cancel());
});

test("Module exceptions don't escape poll", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    Module._web_persist_kind = () => { throw new Error("module boom"); };
    const client = createMockClient();
    Bridge.init(Module, { client });
    assert.doesNotThrow(() => Bridge.poll());
});

test("outcome is always integer number", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    Bridge.init(Module, { client });

    Module._setKind(1);
    Module._setTicket(1);
    Bridge.poll();
    await tick();
    client._rejectLoad({ code: "invalid_response" });
    await tick();

    const calls = Module._completeCalls();
    assert.equal(typeof calls[0].outcome, "number");
    assert.equal(Number.isInteger(calls[0].outcome), true);
});

test("init returns false with null Module", () => {
    const Bridge = freshBridge();
    assert.equal(Bridge.init(null), false);
});

test("init returns false with Module missing _web_persist_kind", () => {
    const Bridge = freshBridge();
    const Module = { _web_persist_ticket: () => 0 };
    assert.equal(Bridge.init(Module), false);
});

test("init returns true with injected client", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    assert.equal(Bridge.init(Module, { client }), true);
});

test("init returns false when SpacePersistence.createClient is missing", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    globalThis.SpacePersistence = {};
    assert.equal(Bridge.init(Module), false);
    delete globalThis.SpacePersistence;
});

test("SCORE LIST ok: feeds entries and completes with 1", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    Bridge.init(Module, { client });

    Module._setKind(4);
    Module._setTicket(7);
    Bridge.poll();
    await tick();

    client._resolveScores([
        { name: "AAA", score: 1000, position: 1 },
        { name: "ZZZ", score: 500, position: 2 }
    ]);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 1);
    assert.equal(calls[0].ticket, 7);
    assert.equal(calls[0].outcome, 1);
    assert.equal(client._scores().length, 1);
});

test("SCORE LIST error maps to -1", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    Bridge.init(Module, { client });

    Module._setKind(4);
    Module._setTicket(3);
    Bridge.poll();
    await tick();

    client._rejectScores({ code: "network_error" });
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 1);
    assert.equal(calls[0].outcome, -1);
});

test("SCORE LIST late response against new ticket is ignored", async () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const client = createMockClient();
    Bridge.init(Module, { client });

    Module._setKind(4);
    Module._setTicket(11);
    Bridge.poll();
    await tick();

    Module._setKind(4);
    Module._setTicket(22);
    Bridge.poll();
    await tick();

    client._scores()[0].resolve([{ name: "AAA", score: 1000, position: 1 }]);
    await tick();

    const calls = Module._completeCalls();
    assert.equal(calls.length, 0);
});

test("init tolerates client exceptions", () => {
    const Bridge = freshBridge();
    const Module = createMockModule();
    const throwingClient = {
        load: () => { throw new Error("boom"); },
        cancel: () => { throw new Error("boom2"); },
        getState: () => { throw new Error("boom3"); }
    };
    assert.equal(Bridge.init(Module, { client: throwingClient }), true);
    assert.doesNotThrow(() => Bridge.poll());
    assert.doesNotThrow(() => Bridge.cancel());
});
