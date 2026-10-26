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
        var keys = {KeyW: 0, KeyS: 1, KeyA: 2, KeyD: 3, Space: 4, Enter: 5, NumpadEnter: 5};
        var pressed = new Set();
        function reset() { pressed.clear(); Module._web_reset(); }
        window.addEventListener("keydown", function (event) {
            if (!Object.prototype.hasOwnProperty.call(keys, event.code) ||
                event.ctrlKey || event.metaKey || event.altKey) return;
            event.preventDefault();
            if (event.repeat || pressed.has(event.code)) return;
            pressed.add(event.code);
            Module._web_input(keys[event.code], 1);
        });
        window.addEventListener("keyup", function (event) {
            if (!pressed.has(event.code)) return;
            event.preventDefault();
            pressed.delete(event.code);
            Module._web_input(keys[event.code], 0);
        });
        window.addEventListener("blur", reset);
        document.addEventListener("visibilitychange", function () {
            if (document.hidden) reset();
        });
        statusNode.textContent = "ENTER: iniciar · W/S: mover · A/D: a partir do nível 2 · Espaço: disparar. Sem áudio ou gravação nesta versão.";
    }
};
