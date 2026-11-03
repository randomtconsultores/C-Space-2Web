#!/usr/bin/env bash
# run-local.sh — arranque local do backend Flask para teste HTTPS de LOAD/SAVE.
#
# Executa o Flask somente em 127.0.0.1:8002, com SPACE_API_ORIGIN apontando
# para a origem HTTPS efetiva (https://cspace.test:8443) que o Caddy expõe.
# O segredo da API e' gerado aleatoriamente por execucao, apenas na memoria
# do processo (nao e' impresso nem gravado em ficheiro).
#
# Pre-requisitos (do utilizador, no Ubuntu):
#   - make -f web/Makefile all  (ja executado; web/build/ completo)
#   - repositorio do C-Space-2Web na branch migration/wasm
#
# Variaveis de ambiente opcionais (sobrescrevem valores por defeito):
#   CSPACE_REPO        diretorio do repositorio (def: raiz deduzida do script)
#   CSPACE_PORT        porta do Flask (def: 8002)
#   CSPACE_DB_DIR      diretorio da base SQLite (def: ~/.local/share/cspace)
set -euo pipefail

CSPACE_REPO="${CSPACE_REPO:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
CSPACE_PORT="${CSPACE_PORT:-8002}"
CSPACE_DB_DIR="${CSPACE_DB_DIR:-$HOME/.local/share/cspace}"
CSPACE_API_ORIGIN="https://cspace.test:8443"
CSPACE_WEB_ROOT="$CSPACE_REPO/web/build"

if [ ! -d "$CSPACE_REPO" ]; then
    echo "ERRO: repositorio nao encontrado em $CSPACE_REPO" >&2
    exit 1
fi
if [ ! -d "$CSPACE_WEB_ROOT" ] || [ ! -f "$CSPACE_WEB_ROOT/space.js" ]; then
    echo "ERRO: web/build/ incompleto em $CSPACE_WEB_ROOT (execute 'make -f web/Makefile all')" >&2
    exit 1
fi

mkdir -p "$CSPACE_DB_DIR"
CSPACE_DATABASE="$CSPACE_DB_DIR/game.sqlite3"

# Segredo aleatorio (>= 32 bytes), apenas na memoria deste processo.
SPACE_API_SECRET="$(head -c 64 /dev/urandom | base64 | tr -dc 'A-Za-z0-9' | head -c 48)"
if [ "${#SPACE_API_SECRET}" -lt 32 ]; then
    echo "ERRO: nao foi possível gerar SPACE_API_SECRET" >&2
    exit 1
fi

export SPACE_API_ORIGIN
export SPACE_API_SECRET
export SPACE_DATABASE="$CSPACE_DATABASE"
export SPACE_WEB_ROOT="$CSPACE_WEB_ROOT"

echo "[run-local] SPACE_API_ORIGIN=$SPACE_API_ORIGIN"
echo "[run-local] SPACE_DATABASE=$SPACE_DATABASE"
echo "[run-local] SPACE_WEB_ROOT=$SPACE_WEB_ROOT"
echo "[run-local] Flask escutando em 127.0.0.1:$CSPACE_PORT (somente loopback)"
echo "[run-local] SPACE_API_SECRET gerado (nao impresso)"

# Inicializa o esquema SQLite explicitamente, so se a base ainda nao existir.
python3 - "$CSPACE_REPO" "$CSPACE_DATABASE" <<'PY'
import os, sys
repo, db_path = sys.argv[1], sys.argv[2]
sys.path.insert(0, repo)
from server.db import init_database, absolute_path
absolute_path(db_path)
if not os.path.exists(db_path):
    init_database(db_path)
    print(f"[run-local] schema inicializado em {db_path}")
else:
    print(f"[run-local] schema ja existe em {db_path} (nao recriado)")
PY

# Arranque do Flask somente em loopback.
python3 - "$CSPACE_REPO" "$CSPACE_PORT" <<'PY'
import os, sys
repo, port = sys.argv[1], int(sys.argv[2])
sys.path.insert(0, repo)
from server.app import create_app
app = create_app()  # le SPACE_API_ORIGIN / SPACE_API_SECRET / etc. do ambiente
app.run(host="127.0.0.1", port=port)
PY
