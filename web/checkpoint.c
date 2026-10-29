#include "checkpoint.h"
#include "levels.h"
#include <limits.h>
#include <string.h>
_Static_assert(LONG_MAX >= 2147483647L, "Checkpoint requires at least 32-bit long");
_Static_assert(INT_MAX >= 2147483647, "Checkpoint tickets require at least 32-bit int");

int web_checkpoint_validate(const WebCheckpoint *cp)
{
    if (!cp || cp->format_version != WEB_CHECKPOINT_VERSION ||
        cp->level_index < 0 || cp->level_index >= WEB_CHECKPOINT_LEVELS ||
        cp->lives < 1 || cp->lives > MAX_VIDAS_JOGADOR ||
        cp->score < 0 || cp->score > WEB_CHECKPOINT_SCORE_MAX)
        return WEB_CHECKPOINT_INVALID;
    return WEB_CHECKPOINT_OK;
}

int web_checkpoint_capture(const Player *player, int next_level, WebCheckpoint *output)
{
    if (!player || !output) return WEB_CHECKPOINT_INVALID;
    const WebCheckpoint cp = {WEB_CHECKPOINT_VERSION, next_level, player->vidas, player->pontuacao};
    if (web_checkpoint_validate(&cp) != WEB_CHECKPOINT_OK) return WEB_CHECKPOINT_INVALID;
    *output = cp;
    return WEB_CHECKPOINT_OK;
}

int web_checkpoint_restore(const WebCheckpoint *cp, Player *output, int *level)
{
    if (!output || !level || web_checkpoint_validate(cp) != WEB_CHECKPOINT_OK)
        return WEB_CHECKPOINT_INVALID;
    if (cp->level_index >= MAX_NIVEIS) return WEB_CHECKPOINT_UNAVAILABLE;
    const int length = configuracao_niveis[cp->level_index].comprimento_mapa;
    if (length < LARGURA_TELA || length > LARGURA_MAPA)
        return WEB_CHECKPOINT_UNAVAILABLE;
    Player next;
    memset(&next, 0, sizeof next);
    next.x = 2;
    next.y = 7;
    next.vidas = cp->lives;
    next.pontuacao = cp->score;
    /* Score cap leaves room for the next 10000-point reward on Wasm32. */
    next.proximo_patamar_pontos = (cp->score / 10000L + 1L) * 10000L;
    *output = next;
    *level = cp->level_index;
    return WEB_CHECKPOINT_OK;
}

int web_checkpoint_gate_begin(WebCheckpointGate *gate)
{
    if (!gate || gate->pending != 0 || gate->serial < 0 || gate->serial == INT_MAX)
        return 0;
    gate->pending = ++gate->serial;
    return gate->pending;
}

void web_checkpoint_gate_cancel(WebCheckpointGate *gate)
{
    if (gate) gate->pending = 0;
}

int web_checkpoint_gate_consume(WebCheckpointGate *gate, int ticket)
{
    if (!gate || ticket <= 0 || gate->pending != ticket) return 0;
    gate->pending = 0;
    return 1;
}
