# Teste local HTTPS — aceitacao de LOAD/SAVE (C-Space-2Web, migration/wasm)

Objetivo: servir frontend e API na mesma origem `https://cspace.test:8443`
sem reescrever a cabeca `Origin`, para o utilizador testar no browser do
Ubuntu: LOAD sem save, SAVE entre niveis, recarregar a pagina e carregar.

Este diretorio contem apenas a preparacao do teste local; o codigo do jogo e da API nao e alterado. Fica em
`docs/local-https/`.

## Requisitos do sistema (Ubuntu)

Instalar (acoes do utilizador; o container nao tem estes pacotes):

```bash
sudo apt update
sudo apt install -y mkcert libnss3-tools caddy
```

- `caddy`: binario `caddy` (versao 2.x).
- `mkcert` e `libnss3-tools`: para gerar a CA local e o certificado de
  `cspace.test`.

## Certificado (fora do repositorio)

```bash
mkcert -install
mkcert cspace.test
```

Isso gera `cspace.test.pem` e `cspace.test-key.pem` no diretorio corrente.
Para que o Caddyfile encontre os ficheiros como `cert.pem`/`key.pem`:

```bash
ln -sf "$(pwd)/cspace.test.pem"     cert.pem
ln -sf "$(pwd)/cspace.test-key.pem" key.pem
```

Os certificados e chaves nunca sao inseridos no repositorio.

## /etc/hosts (acao do utilizador)

```bash
echo "127.0.0.1 cspace.test" | sudo tee -a /etc/hosts
```

O aviso da tarefa proibe o assistente de editar `/etc/hosts`: o utilizador
faz este passo.

## Build do frontend (Wasm)

Do diretorio raiz do repositorio (`C-Space-2Web`, branch `migration/wasm`,
Emscripten ativado):

```bash
make -f web/Makefile all
```

Confirmar que `web/build/` contem: `space.js`, `space.wasm`,
`index.html`, `browser.js`, `persistence.js`, `persistence_bridge.js` e
`sounds/`.

## Arranque do backend (run-local.sh)

Num terminal:

```bash
chmod +x docs/local-https/run-local.sh
./docs/local-https/run-local.sh
```

O que o script faz:
- gera um `SPACE_API_SECRET` aleatorio (>= 32 bytes) apenas na memoria do
  processo (nao imprime, nao grava em ficheiro);
- usa a base SQLite em `~/.local/share/cspace/game.sqlite3` (fora da arvore
  pública), inicializando o esquema so se ainda nao existir;
- define `SPACE_API_ORIGIN=https://cspace.test:8443`;
- define `SPACE_WEB_ROOT` para `web/build`;
- arranca o Flask apenas em `127.0.0.1:8002` (nao `0.0.0.0`).

O Flask fica escutando em `127.0.0.1:8002`. O Caddy faz o terminaison TLS e
o reverse proxy para esta porta.

## Arranque do Caddy

Num outro terminal, no diretorio que contem o `Caddyfile` e os ficheiros
`cert.pem`/`key.pem`:

```bash
caddy run --config Caddyfile
```

O Caddy ouve em `cspace.test:8443` (HTTPS) e faz `reverse_proxy` para
`127.0.0.1:8002`, preservando a cabeca `Origin` do browser.

## Teste no browser

Abrir: `https://cspace.test:8443`

Fluxo de aceitacao de LOAD/SAVE:
1. LOAD sem save existente -> deve retornar sem erro / sem checkpoint.
2. Joga ate passar de nivel e faz SAVE.
3. Recarrega a pagina.
4. LOAD -> deve carregar o checkpoint gravado.

No painel de rede (Network) do browser, confirmar:
- `POST /api/v1/session` com `Origin: https://cspace.test:8443` -> `201`
- `GET /api/v1/health` -> `200`
- Qualquer pedido com `Origin: http://localhost:8002` -> `403 origin_rejected`
  (esperado; validacao de origem nao e relaxada).

## Aviso: Chrome/Chromium em snap ou Flatpak

Versões snap/Flatpak do Chrome/Chromium podem nao confiar na CA do sistema
instalada pelo `mkcert -install`, resultando em erro `NET::ERR_CERT_AUTHORITY_INVALID`.

O que fazer:
- usar o Chromium/Chrome instalado via `.deb` (repositorio oficial do Google)
  ou via `apt`, que le a CA do sistema; ou
- importar a CA raiz do mkcert (`$(mkcert -CAROOT)/rootCA.pem`) nas
  configuracoes de certificados do navegador.

## Como parar tudo

```bash
# parar o Caddy
C-c   (ou: caddy stop --config Caddyfile)

# parar o run-local.sh
C-c   (no terminal do Flask)
```

## Remocao (limpeza completa)

```bash
# remover a entrada de hosts (acao do utilizador)
sudo sed -i '/cspace.test/d' /etc/hosts

# desinstalar a CA local do mkcert
mkcert -uninstall

# remover certificados gerados
rm -f cert.pem key.pem cspace.test.pem cspace.test-key.pem

# remover base de dados local (opcional)
rm -f ~/.local/share/cspace/game.sqlite3
```

## Variaveis de ambiente do run-local.sh (sobrescritaveis)

- `CSPACE_REPO` — diretorio do repositorio (def: raiz deduzida da localizacao do script)
- `CSPACE_PORT` — porta do Flask (def: `8002`)
- `CSPACE_DB_DIR` — diretorio da SQLite (def: `~/.local/share/cspace`)

## Notas de seguranca (mantidas)

- A validacao de `Origin`/CSRF nao e alterada nem relaxada.
- `localhost` nao e adicionado a lista de origens permitidas.
- A origem nao e inferida de `Host` nem de `X-Forwarded-*`.
- O Flask escuta apenas em `127.0.0.1`; o Caddy expoe o HTTPS externo.
