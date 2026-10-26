"use strict";
var statusNode = document.getElementById("status");
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
        };
        var special = {Enter: 5, NumpadEnter: 5, Escape: 6, ArrowUp: 7, ArrowDown: 8};
        var pressed = new Map();
        var heldCounts = new Uint8Array(10);
        function reset() { pressed.clear(); heldCounts.fill(0); Module._web_reset(); }
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
            pressed.set(event.code, mapped);
            mapped.forEach(function (action) {
                if (heldCounts[action]++ === 0) Module._web_input(action, 1);
            });
        });
        window.addEventListener("keyup", function (event) {
            if (!pressed.has(event.code)) return;
            event.preventDefault();
            var mapped = pressed.get(event.code);
            pressed.delete(event.code);
            mapped.forEach(function (action) {
                if (--heldCounts[action] === 0) Module._web_input(action, 0);
            });
        });
        window.addEventListener("blur", reset);
        document.addEventListener("visibilitychange", function () {
            if (document.hidden) reset();
        });
        statusNode.textContent = "Menu: W/S ou setas, Enter seleciona, Esc recua/cancela. Jogo: Q pede perda de vida (respeita imunidade). Sem áudio ou gravação.";
    }
};
