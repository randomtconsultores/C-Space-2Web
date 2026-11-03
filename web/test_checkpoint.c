#include "checkpoint.h"
#include "levels.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static Player sentinel(void)
{
    Player p;
    memset(&p, 0, sizeof p);
    p.x = 19; p.y = 8; p.vidas = 7; p.imunidade = 33;
    p.pontuacao = 456; p.proximo_patamar_pontos = 10000;
    p.tiro[0].ativo = 1;
    return p;
}

static void test_validation(void)
{
    const WebCheckpoint valid[] = {
        {1, 0, 1, 0}, {1, 2, 9, WEB_CHECKPOINT_SCORE_MAX}, {1, 3, 3, 10}
    };
    for (unsigned i = 0; i < sizeof valid / sizeof valid[0]; ++i)
        assert(web_checkpoint_validate(&valid[i]) == WEB_CHECKPOINT_OK);
    const WebCheckpoint invalid[] = {
        {0, 0, 3, 0}, {2, 0, 3, 0}, {1, -1, 3, 0}, {1, 4, 3, 0},
        {1, 0, 0, 0}, {1, 0, 10, 0}, {1, 0, 3, -1},
        {1, 0, 3, WEB_CHECKPOINT_SCORE_MAX + 1L}
    };
    assert(web_checkpoint_validate(NULL) == WEB_CHECKPOINT_INVALID);
    for (unsigned i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
        Player p = sentinel(), before;
        memcpy(&before, &p, sizeof p);
        int level = 2;
        assert(web_checkpoint_validate(&invalid[i]) == WEB_CHECKPOINT_INVALID);
        assert(web_checkpoint_restore(&invalid[i], &p, &level) == WEB_CHECKPOINT_INVALID);
        assert(memcmp(&p, &before, sizeof p) == 0 && level == 2);
    }
    puts("PASS: DTO bounds and invalid restore preserve outputs");
}

static void test_capture(void)
{
    Player p = sentinel(), before;
    memcpy(&before, &p, sizeof p);
    WebCheckpoint cp;
    assert(web_checkpoint_capture(&p, 3, &cp) == WEB_CHECKPOINT_OK);
    assert(cp.format_version == 1 && cp.level_index == 3 && cp.lives == 7 && cp.score == 456);
    assert(memcmp(&p, &before, sizeof p) == 0);
    WebCheckpoint unchanged;
    memcpy(&unchanged, &cp, sizeof cp);
    assert(web_checkpoint_capture(&p, 4, &cp) == WEB_CHECKPOINT_INVALID);
    assert(memcmp(&cp, &unchanged, sizeof cp) == 0);
    p.vidas = 0;
    assert(web_checkpoint_capture(&p, 0, &cp) == WEB_CHECKPOINT_INVALID);
    assert(memcmp(&cp, &unchanged, sizeof cp) == 0);
    assert(web_checkpoint_capture(NULL, 0, &cp) == WEB_CHECKPOINT_INVALID);
    assert(web_checkpoint_capture(&p, 0, NULL) == WEB_CHECKPOINT_INVALID);
    puts("PASS: capture copies next-level fields only and rejects Game Over");
}

static void test_restore(void)
{
    const long scores[] = {0, 9999, 10000, 23456, WEB_CHECKPOINT_SCORE_MAX};
    for (int stage = 0; stage < 3; ++stage) {
        for (unsigned i = 0; i < sizeof scores / sizeof scores[0]; ++i) {
            const WebCheckpoint cp = {1, stage, 4, scores[i]};
            Player p = sentinel();
            int level = -1;
            assert(web_checkpoint_restore(&cp, &p, &level) == WEB_CHECKPOINT_OK);
            assert(level == stage && p.x == 2 && p.y == 7 && p.vidas == 4);
            assert(p.pontuacao == scores[i] && p.imunidade == 0);
            assert(p.proximo_patamar_pontos > p.pontuacao);
            assert(p.proximo_patamar_pontos <= 2147483647L);
            assert(p.proximo_patamar_pontos == (scores[i] / 10000L + 1L) * 10000L);
            for (int j = 0; j < MAX_TIROS; ++j)
                assert(p.tiro[j].ativo == 0 && p.tiro[j].x == 0 && p.tiro[j].y == 0);
        }
    }
    puts("PASS: configured levels restore fresh player and safe reward threshold");
}

static void test_unavailable_and_nulls(void)
{
    WebCheckpoint cp = {1, 3, 3, 100};
    Player p = sentinel(), before;
    memcpy(&before, &p, sizeof p);
    int level = 1;
    assert(configuracao_niveis[3].comprimento_mapa == 0);
    assert(web_checkpoint_restore(&cp, &p, &level) == WEB_CHECKPOINT_UNAVAILABLE);
    assert(memcmp(&p, &before, sizeof p) == 0 && level == 1);
    cp.level_index = 0;
    const int old = configuracao_niveis[0].comprimento_mapa;
    const int bad_lengths[] = {-1, LARGURA_TELA - 1, LARGURA_MAPA + 1};
    for (unsigned i = 0; i < sizeof bad_lengths / sizeof bad_lengths[0]; ++i) {
        configuracao_niveis[0].comprimento_mapa = bad_lengths[i];
        assert(web_checkpoint_restore(&cp, &p, &level) == WEB_CHECKPOINT_UNAVAILABLE);
        assert(memcmp(&p, &before, sizeof p) == 0 && level == 1);
    }
    configuracao_niveis[0].comprimento_mapa = old;
    assert(web_checkpoint_restore(NULL, &p, &level) == WEB_CHECKPOINT_INVALID);
    assert(web_checkpoint_restore(&cp, NULL, &level) == WEB_CHECKPOINT_INVALID);
    assert(web_checkpoint_restore(&cp, &p, NULL) == WEB_CHECKPOINT_INVALID);
    assert(memcmp(&p, &before, sizeof p) == 0 && level == 1);
    puts("PASS: unavailable fourth level and null arguments do not destroy state");
}

static void test_tickets(void)
{
    WebCheckpointGate gate = {0, 0};
    assert(web_checkpoint_gate_begin(NULL) == 0);
    assert(web_checkpoint_gate_consume(NULL, 1) == 0);
    web_checkpoint_gate_cancel(NULL);
    int first = web_checkpoint_gate_begin(&gate);
    assert(first == 1 && web_checkpoint_gate_begin(&gate) == 0);
    assert(!web_checkpoint_gate_consume(&gate, 0));
    assert(!web_checkpoint_gate_consume(&gate, first + 1));
    assert(web_checkpoint_gate_consume(&gate, first));
    assert(!web_checkpoint_gate_consume(&gate, first));
    int second = web_checkpoint_gate_begin(&gate);
    web_checkpoint_gate_cancel(&gate);
    int third = web_checkpoint_gate_begin(&gate);
    assert(third > second && second > first);
    assert(!web_checkpoint_gate_consume(&gate, second));
    assert(web_checkpoint_gate_consume(&gate, third));
    gate.serial = INT_MAX - 1;
    assert(web_checkpoint_gate_begin(&gate) == INT_MAX);
    assert(web_checkpoint_gate_consume(&gate, INT_MAX));
    assert(web_checkpoint_gate_begin(&gate) == 0 && gate.serial == INT_MAX);
    puts("PASS: one-shot tickets, cancellation, stale replies and no counter wrap");
}

int main(void)
{
    test_validation(); test_capture(); test_restore();
    test_unavailable_and_nulls(); test_tickets();
    return 0;
}
