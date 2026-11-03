// web/persistence_bridge.js
// Ponte assíncrona JS ↔ C para Save/LOAD via SpacePersistence
// Script clássico (UMD), expõe SpacePersistenceBridge como objeto frozen.

(function (root, factory) {
    const api = factory(root);
    if (typeof module === "object" && module.exports) module.exports = api;
    else root.SpacePersistenceBridge = api;
})(globalThis, function (root) {
    "use strict";

    let moduleInstance = null;
    let client = null;
    let initialized = false;
    let activeTicket = 0;
    let activeKind = 0;            // 1=LOAD, 2=SAVE
    let operationPromise = null;
    let lastHandledTicket = 0;     // último ticket para o qual complete() foi chamado

    // Mapa de erros sem herança de prototype
    const ERROR_MAP = Object.create(null);
    ERROR_MAP.cancelled = -3;
    ERROR_MAP.invalid_response = -2;
    ERROR_MAP.unsupported_format = -2;
    ERROR_MAP.invalid_snapshot = -2;
    // not_found NÃO entra aqui; null do load -> 0, erro not_found -> -1
    ERROR_MAP.load_required = -1;
    ERROR_MAP.revision_conflict = -1;
    ERROR_MAP.session_expired = -1;
    ERROR_MAP.csrf_rejected = -1;
    ERROR_MAP.session_rate_limited = -1;
    ERROR_MAP.storage_unavailable = -1;
    ERROR_MAP.http_error = -1;
    ERROR_MAP.network_error = -1;
    ERROR_MAP.invalid_configuration = -1;
    ERROR_MAP.unsupported = -1;
    ERROR_MAP.busy = -1;

    function mapErrorToOutcome(error) {
        if (!error || typeof error.code !== "string") return -1;
        if (error.code === "cancelled") return -3;
        return ERROR_MAP[error.code] ?? -1;
    }

    function safeCall(fn, fallback) {
        try {
            return fn();
        } catch (e) {
            return fallback;
        }
    }

    function resetOperation() {
        activeTicket = 0;
        activeKind = 0;
        operationPromise = null;
    }

    function complete(ticket, outcome, fv, li, lv, sc) {
        // Verifica se o C ainda espera este ticket/kind
        if (!moduleInstance ||
            typeof moduleInstance._web_persist_complete !== "function" ||
            moduleInstance._web_persist_kind() !== activeKind ||
            moduleInstance._web_persist_ticket() !== ticket) {
            return;
        }
        // Evita complete duplicado para o mesmo ticket
        if (ticket === lastHandledTicket) return;
        lastHandledTicket = ticket;

        safeCall(() => moduleInstance._web_persist_complete(ticket, outcome, fv, li, lv, sc));
    }

    function startLoad(ticket) {
        if (!client) return;
        operationPromise = Promise.resolve().then(() => {
            // Verifica novamente antes de chamar
            if (activeTicket !== ticket || activeKind !== 1) return;
            return safeCall(() => client.load());
        })
        .then(cp => {
            if (activeTicket !== ticket || activeKind !== 1) return;
            if (cp === null) {
                complete(ticket, 0, 0, 0, 0, 0);
            } else {
                complete(ticket, 1, cp.format_version, cp.level_index, cp.lives, cp.score);
            }
        })
        .catch(err => {
            if (activeTicket !== ticket || activeKind !== 1) return;
            const outcome = mapErrorToOutcome(err);
            // cancelled só se o C ainda espera este ticket
            if (outcome === -3 && (activeKind !== 1 || activeTicket !== ticket)) return;
            complete(ticket, outcome, 0, 0, 0, 0);
        })
        .finally(() => {
            if (activeTicket === ticket && activeKind === 1) resetOperation();
        });
    }

    function startSave(ticket) {
        if (!client) return;
        // Lê os 4 campos com verificação de ticket/kind
        const fv = safeCall(() => {
            if (activeTicket !== ticket || activeKind !== 2) return -1;
            return moduleInstance._web_persist_field(ticket, 0);
        }, -1);
        const li = safeCall(() => {
            if (activeTicket !== ticket || activeKind !== 2) return -1;
            return moduleInstance._web_persist_field(ticket, 1);
        }, -1);
        const lv = safeCall(() => {
            if (activeTicket !== ticket || activeKind !== 2) return -1;
            return moduleInstance._web_persist_field(ticket, 2);
        }, -1);
        const sc = safeCall(() => {
            if (activeTicket !== ticket || activeKind !== 2) return -1;
            return moduleInstance._web_persist_field(ticket, 3);
        }, -1);

        if (fv === -1 || li === -1 || lv === -1 || sc === -1) {
            complete(ticket, -2, 0, 0, 0, 0);
            resetOperation();
            return;
        }

        const snapshot = { format_version: fv, level_index: li, lives: lv, score: sc };

        const doSave = () => Promise.resolve().then(() => {
            // Verifica ticket/kind antes do save
            if (activeTicket !== ticket || activeKind !== 2) return;
            return safeCall(() => client.save(snapshot));
        })
        .then(cp => {
            if (activeTicket !== ticket || activeKind !== 2) return;
            complete(ticket, 1, cp.format_version, cp.level_index, cp.lives, cp.score);
        })
        .catch(err => {
            if (activeTicket !== ticket || activeKind !== 2) return;
            const outcome = mapErrorToOutcome(err);
            if (outcome === -3 && (activeKind !== 2 || activeTicket !== ticket)) return;
            complete(ticket, outcome, 0, 0, 0, 0);
        })
        .finally(() => {
            if (activeTicket === ticket && activeKind === 2) resetOperation();
        });

        // Verifica revisão; se null, faz load primeiro
        const state = safeCall(() => {
            if (activeTicket !== ticket || activeKind !== 2) return { revision: null };
            return client.getState();
        }, { revision: null });

        if (state.revision === null) {
            operationPromise = Promise.resolve().then(() => {
                if (activeTicket !== ticket || activeKind !== 2) return;
                return safeCall(() => client.load());
            })
            .then(() => doSave())
            .catch(err => {
                if (activeTicket !== ticket || activeKind !== 2) return;
                const outcome = mapErrorToOutcome(err);
                if (outcome === -3 && (activeKind !== 2 || activeTicket !== ticket)) return;
                complete(ticket, outcome, 0, 0, 0, 0);
                resetOperation();
            });
        } else {
            operationPromise = doSave();
        }
    }

    function startScoreList(ticket) {
        if (!client) return;
        operationPromise = Promise.resolve().then(() => {
            if (activeTicket !== ticket || activeKind !== 4) return;
            return safeCall(() => client.scores());
        })
        .then(entries => {
            if (activeTicket !== ticket || activeKind !== 4) return;
            let i = 0;
            for (const entry of entries) {
                if (activeTicket !== ticket || activeKind !== 4) return;
                safeCall(() => moduleInstance._web_persist_score_feed_entry(
                    ticket, i, entry.position, entry.score));
                const name = String(entry.name);
                for (let ci = 0; ci < 7; ci++) {
                    safeCall(() => moduleInstance._web_persist_score_feed_name_char(
                        ticket, i, ci, ci < name.length ? name.charCodeAt(ci) : 0));
                }
                i++;
            }
            complete(ticket, 1, 0, 0, 0, 0);
        })
        .catch(err => {
            if (activeTicket !== ticket || activeKind !== 4) return;
            const outcome = mapErrorToOutcome(err);
            if (outcome === -3 && (activeKind !== 4 || activeTicket !== ticket)) return;
            complete(ticket, outcome, 0, 0, 0, 0);
        })
        .finally(() => {
            if (activeTicket === ticket && activeKind === 4) resetOperation();
        });
    }

    function startScoreSubmit(ticket) {
        if (!client) return;
        // Lê o nome (caractere a caractere, ate 7) e a pontuação do C
        let name = "";
        for (let ci = 0; ci < 7; ci++) {
            const code = safeCall(() => moduleInstance._web_persist_submit_name_char(ticket, ci), -1);
            if (code === -1 || code === 0) break;
            name += String.fromCharCode(code);
        }
        const score = safeCall(() => moduleInstance._web_persist_submit_score(ticket), -1);
        if (name === "" || score === -1) {
            complete(ticket, -2, 0, 0, 0, 0);
            resetOperation();
            return;
        }
        operationPromise = Promise.resolve().then(() => {
            if (activeTicket !== ticket || activeKind !== 3) return;
            return safeCall(() => client.submitScore(name, score));
        })
        .then(result => {
            if (activeTicket !== ticket || activeKind !== 3) return;
            complete(ticket, 1, result.position, 0, 0, 0);
        })
        .catch(err => {
            if (activeTicket !== ticket || activeKind !== 3) return;
            const outcome = mapErrorToOutcome(err);
            if (outcome === -3 && (activeKind !== 3 || activeTicket !== ticket)) return;
            complete(ticket, outcome, 0, 0, 0, 0);
        })
        .finally(() => {
            if (activeTicket === ticket && activeKind === 3) resetOperation();
        });
    }

    const SpacePersistenceBridge = Object.freeze({
        init(Module, options) {
            if (initialized) return false; // rejeita segundo init
            if (!Module || typeof Module._web_persist_kind !== "function") return false;

            // options pode ser undefined/null
            const opts = options || {};
            if (!opts.client) {
                if (!root.SpacePersistence || typeof root.SpacePersistence.createClient !== "function") {
                    return false; // falha controlada
                }
                client = root.SpacePersistence.createClient();
            } else {
                client = opts.client;
            }

            moduleInstance = Module;
            initialized = true;
            resetOperation();
            lastHandledTicket = 0;
            return true;
        },

        poll() {
            if (!initialized || !moduleInstance || !client) return;

            safeCall(() => {
                const kind = safeCall(() => moduleInstance._web_persist_kind(), 0);
                const ticket = safeCall(() => moduleInstance._web_persist_ticket(), 0);

                // Nenhum pedido ativo no C
                if (kind === 0 || ticket === 0) {
                    if (activeTicket !== 0) {
                        // C cancelou: cancela cliente, descarta operação, NÃO chama complete
                        safeCall(() => client.cancel());
                        resetOperation();
                    }
                    return;
                }

                // Se este ticket já foi tratado, não arrancar nova operação
                if (ticket === lastHandledTicket) return;

                // Novo ticket ou mudança de kind/ticket
                if (ticket !== activeTicket || kind !== activeKind) {
                    if (activeTicket !== 0) {
                        safeCall(() => client.cancel());
                    }
                    resetOperation();
                    activeTicket = ticket;
                    activeKind = kind;
                }

                // Se já há operação em curso para este ticket, não relança (idempotência)
                if (operationPromise) return;

                if (kind === 1) {
                    startLoad(ticket);
                } else if (kind === 2) {
                    startSave(ticket);
                } else if (kind === 3) {
                    startScoreSubmit(ticket);
                } else if (kind === 4) {
                    startScoreList(ticket);
                }
            });
        },

        cancel() {
            if (!initialized) return;
            safeCall(() => client?.cancel());
            resetOperation();
        }
    });

    return SpacePersistenceBridge;
});
