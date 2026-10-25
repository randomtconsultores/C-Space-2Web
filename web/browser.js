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
        statusNode.textContent = "Wasm inicializado; a gerar o mapa…";
    },
    postRun: [function () {
        try {
            var pointer = Module._web_buffer();
            var size = Module._web_buffer_size();
            var expected = (Module._web_width() + 1) * Module._web_height();
            var heap = Module.HEAPU8;
            if (size !== expected || pointer < 0 || pointer > heap.length - size)
                throw new Error("Dimensões ou endereço do framebuffer inválidos");
            var text = new TextDecoder("utf-8", {fatal: true}).decode(
                heap.subarray(pointer, pointer + size));
            document.getElementById("screen").textContent = text;
            statusNode.textContent = "Mapa gerado pelo C/Wasm. Jogabilidade ainda não integrada.";
        } catch (error) {
            statusNode.textContent = "Falha na apresentação: " + String(error);
            console.error(error);
        }
    }]
};
