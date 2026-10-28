"use strict";
(function (root, factory) {
    const api = factory(root);
    if (typeof module === "object" && module.exports) module.exports = api;
    else root.SpacePersistence = api;
})(globalThis, function (root) {
    const SNAPSHOT = ["format_version", "level_index", "lives", "score"];
    const CHECKPOINT = SNAPSHOT.concat(["revision", "updated_at"]);
    const ERRORS = {
        400: ["bad_json", "bad_request"], 401: ["session_required", "session_invalid", "session_expired"],
        403: ["origin_rejected", "csrf_rejected"], 404: ["not_found", "checkpoint_missing"],
        405: ["method_not_allowed"], 409: ["revision_conflict"], 413: ["payload_too_large"],
        415: ["json_required"], 422: ["invalid_payload", "unexpected_query"],
        429: ["session_capacity", "session_rate_limited"], 503: ["storage_unavailable"]
    };
    class PersistenceError extends Error {
        constructor(code, status = 0, uncertain = false) {
            super(code);
            this.name = "PersistenceError";
            this.code = code;
            this.status = status;
            this.uncertain = uncertain;
        }
    }
    function record(value, keys) {
        return value !== null && typeof value === "object" && !Array.isArray(value) &&
            Object.keys(value).length === keys.length &&
            keys.every(key => Object.prototype.hasOwnProperty.call(value, key));
    }
    function integer(value, low, high) { return Number.isSafeInteger(value) && value >= low && value <= high; }
    function snapshot(value) {
        if (!record(value, SNAPSHOT) || value.format_version !== 1 ||
            !integer(value.level_index, 0, 3) || !integer(value.lives, 1, 9) ||
            !integer(value.score, 0, 2147473647)) throw new PersistenceError("invalid_snapshot");
        return Object.freeze({format_version: 1, level_index: value.level_index, lives: value.lives, score: value.score});
    }
    function discard(body) {
        if (body) { try { Promise.resolve(body.cancel()).catch(() => {}); } catch (_) {} }
    }
    function createClient(options = {}) {
        if (options === null || typeof options !== "object" || Array.isArray(options) || Object.keys(options).some(key =>
            !["fetch", "timeoutMs", "setTimer", "clearTimer"].includes(key)))
            throw new PersistenceError("invalid_configuration");
        const fetcher = options.fetch || (typeof root.fetch === "function" && root.fetch.bind(root));
        const setTimer = options.setTimer || root.setTimeout.bind(root);
        const clearTimer = options.clearTimer || root.clearTimeout.bind(root);
        const timeout = options.timeoutMs === undefined ? 10000 : options.timeoutMs;
        if (typeof fetcher !== "function" || typeof root.AbortController !== "function" ||
            typeof root.TextDecoder !== "function") throw new PersistenceError("unsupported");
        if (!integer(timeout, 1, 60000) || typeof setTimer !== "function" || typeof clearTimer !== "function")
            throw new PersistenceError("invalid_configuration");
        let csrf = null, revision = null, active = null, epoch = 0;
        function fault(code, ctx, status = 0) { return new PersistenceError(code, status, ctx.writeSent); }
        function live(ctx) { if (!ctx.live) throw ctx.reason || fault("cancelled", ctx); }
        function stop(ctx, code) {
            if (!ctx || !ctx.live) return;
            ctx.reason = fault(code, ctx);
            ctx.live = false;
            if (active === ctx) { active = null; csrf = null; revision = null; ++epoch; }
            clearTimer(ctx.timer);
            ctx.reject(ctx.reason);
            ctx.controller.abort();
            discard(ctx.reader);
        }
        async function run(task) {
            if (active) throw new PersistenceError("busy");
            const ctx = {live: true, writeSent: false, controller: new root.AbortController(), reader: null};
            active = ctx;
            const interrupted = new Promise((resolve, reject) => { ctx.reject = reject; });
            try {
                ctx.timer = setTimer(() => stop(ctx, "timeout"), timeout);
                return await Promise.race([Promise.resolve().then(() => { live(ctx); return task(ctx); }), interrupted]);
            } catch (error) {
                if (active === ctx) { csrf = null; revision = null; ++epoch; }
                throw error instanceof PersistenceError ? error : fault("network_error", ctx);
            } finally {
                ctx.live = false;
                clearTimer(ctx.timer);
                if (active === ctx) active = null;
            }
        }
        async function jsonBody(response, ctx) {
            const status = response.status;
            let reader;
            try {
                const type = (response.headers.get("Content-Type") || "").split(";")[0].trim().toLowerCase();
                const declared = response.headers.get("Content-Length");
                if (type !== "application/json" || (declared !== null &&
                    (!/^\d+$/.test(declared) || Number(declared) > 4096)) || !response.body)
                    throw fault("invalid_response", ctx, status);
                reader = response.body.getReader();
                ctx.reader = reader;
                const bytes = new Uint8Array(4096);
                let used = 0, reads = 0;
                for (;;) {
                    if (++reads > 4097) throw fault("invalid_response", ctx, status);
                    const chunk = await reader.read();
                    live(ctx);
                    if (chunk.done) break;
                    if (used + chunk.value.byteLength > bytes.length) throw fault("invalid_response", ctx, status);
                    bytes.set(chunk.value, used);
                    used += chunk.value.byteLength;
                }
                return JSON.parse(new root.TextDecoder("utf-8", {fatal: true}).decode(bytes.subarray(0, used)));
            } catch (error) {
                discard(reader || response.body);
                throw error instanceof PersistenceError ? error : fault("invalid_response", ctx, status);
            } finally {
                ctx.reader = null;
                if (reader) reader.releaseLock();
            }
        }
        async function request(ctx, path, method, payload, protectedRequest, statuses) {
            live(ctx);
            const headers = {Accept: "application/json"};
            if (payload !== undefined) headers["Content-Type"] = "application/json";
            if (protectedRequest) headers["X-CSRF-Token"] = csrf;
            const init = {method, headers, credentials: "same-origin", mode: "same-origin",
                cache: "no-store", redirect: "error", signal: ctx.controller.signal};
            if (payload !== undefined) init.body = JSON.stringify(payload);
            if (method === "PUT") ctx.writeSent = true;
            const response = await fetcher(path, init);
            if (!ctx.live) { discard(response.body); live(ctx); }
            const value = await jsonBody(response, ctx);
            live(ctx);
            if (response.status >= 400) {
                const code = record(value, ["error"]) && record(value.error, ["code"]) &&
                    (ERRORS[response.status] || []).includes(value.error.code) ? value.error.code : "http_error";
                throw new PersistenceError(code, response.status, ctx.writeSent && (response.status >= 500 || code === "http_error"));
            }
            if (!statuses.includes(response.status)) throw fault("invalid_response", ctx, response.status);
            return value;
        }
        async function session(ctx) {
            if (csrf !== null) return;
            const value = await request(ctx, "/api/v1/session", "POST", {}, false, [200, 201]);
            if (!record(value, ["csrf_token", "expires_at"]) || typeof value.csrf_token !== "string" ||
                !/^[0-9a-f]{64}$/.test(value.csrf_token) || !integer(value.expires_at, 1, Number.MAX_SAFE_INTEGER))
                throw fault("invalid_response", ctx);
            live(ctx);
            csrf = value.csrf_token;
        }
        function checkpoint(value, ctx) {
            if (!record(value, ["checkpoint"]) || !record(value.checkpoint, CHECKPOINT))
                throw fault("invalid_response", ctx);
            const cp = value.checkpoint;
            if (cp.format_version !== 1) throw fault("unsupported_format", ctx);
            let base;
            try { base = snapshot({format_version: cp.format_version, level_index: cp.level_index, lives: cp.lives, score: cp.score}); }
            catch (_) { throw fault("invalid_response", ctx); }
            if (!integer(cp.revision, 1, 2147483647) || !integer(cp.updated_at, 0, Number.MAX_SAFE_INTEGER))
                throw fault("invalid_response", ctx);
            return Object.freeze({...base, revision: cp.revision, updated_at: cp.updated_at});
        }
        return Object.freeze({
            load: () => run(async ctx => {
                await session(ctx);
                let value;
                try { value = await request(ctx, "/api/v1/checkpoint", "GET", undefined, true, [200]); }
                catch (error) {
                    live(ctx);
                    if (error.code !== "checkpoint_missing" || error.status !== 404) throw error;
                    revision = 0;
                    return null;
                }
                const cp = checkpoint(value, ctx);
                live(ctx);
                revision = cp.revision;
                return cp;
            }),
            save: async value => {
                let data;
                try { data = snapshot(value); }
                catch (_) { throw new PersistenceError("invalid_snapshot"); }
                if (active) throw new PersistenceError("busy");
                if (csrf === null || revision === null) throw new PersistenceError("load_required");
                if (revision > 2147483646) throw new PersistenceError("revision_limit");
                return run(async ctx => {
                    const expected = revision;
                    const result = await request(ctx, "/api/v1/checkpoint", "PUT",
                        {...data, expected_revision: expected}, true, [200]);
                    const cp = checkpoint(result, ctx);
                    if (cp.revision !== expected + 1 || SNAPSHOT.some(key => cp[key] !== data[key]))
                        throw fault("invalid_response", ctx);
                    live(ctx);
                    revision = cp.revision;
                    return cp;
                });
            },
            cancel: () => {
                if (active) stop(active, "cancelled");
                else { csrf = null; revision = null; ++epoch; }
            },
            getState: () => Object.freeze({busy: active !== null, revision, epoch})
        });
    }
    return Object.freeze({createClient, PersistenceError});
});
