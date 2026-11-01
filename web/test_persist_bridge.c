#include "persist_bridge.h"
#include "checkpoint.h"
#include "levels.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static int g_test_count = 0;

#define TEST_START() do { printf("  %s... ", __func__); fflush(stdout); } while (0)
#define TEST_END() do { g_test_count++; printf("OK\n"); } while (0)

static Player make_player(int vidas, long score)
{
    Player p;
    memset(&p, 0, sizeof(p));
    p.vidas = vidas;
    p.pontuacao = score;
    p.proximo_patamar_pontos = 10000;
    return p;
}

static void test_init_cleanup(void)
{
    TEST_START();
    web_persist_bridge_init();
    assert(web_persist_kind() == 0);
    assert(web_persist_ticket() == 0);
    web_persist_bridge_cleanup();
    assert(web_persist_kind() == 0);
    TEST_END();
}

static void test_null_pointers(void)
{
    TEST_START();
    web_persist_bridge_init();
    assert(web_persist_begin_save(NULL, 1) == 0);
    assert(web_persist_field(1, 0) == -1);
    assert(web_persist_complete(1, 1, 1, 0, 3, 0) == 0);
    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_load_ok(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_load();
    assert(ticket > 0);
    assert(web_persist_kind() == 1);
    assert(web_persist_ticket() == ticket);

    int consumed = web_persist_complete(ticket, 1, 1, 0, 3, 1000);
    assert(consumed == 1);

    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(kind == 1);
    assert(outcome == 1);
    assert(p.vidas == 3);
    assert(p.pontuacao == 1000);
    assert(level == 0);
    assert(web_persist_kind() == 0);

    assert(web_persist_complete(ticket, 1, 1, 0, 3, 1000) == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_load_not_found(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_load();
    int consumed = web_persist_complete(ticket, 0, 0, 0, 0, 0);
    assert(consumed == 1);
    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(kind == 1);
    assert(outcome == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_load_error(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_load();
    int consumed = web_persist_complete(ticket, -1, 0, 0, 0, 0);
    assert(consumed == 1);
    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_load_cancelled(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_load();
    web_persist_cancel();
    /* Após cancel: kind/ticket voltam a 0, mas take_result ainda devolve o resultado */
    assert(web_persist_kind() == 0);
    assert(web_persist_ticket() == 0);
    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -3);
    /* Resposta tardia após cancelamento é ignorada */
    assert(web_persist_complete(ticket, 1, 1, 0, 3, 1000) == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_load_invalid_snapshot(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_load();
    int consumed = web_persist_complete(ticket, 1, 1, 0, 3, -1);
    assert(consumed == 1);
    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -2);

    ticket = web_persist_begin_load();
    consumed = web_persist_complete(ticket, 1, 1, 0, 0, 1000);
    assert(consumed == 1);
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -2);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_load_unavailable_level3(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_load();
    int consumed = web_persist_complete(ticket, 1, 1, 3, 3, 1000);
    assert(consumed == 1);
    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -4);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_save_ok(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t ticket = web_persist_begin_save(&p, 1);
    assert(ticket > 0);
    assert(web_persist_kind() == 2);
    assert(web_persist_ticket() == ticket);
    assert(web_persist_field(ticket, 0) == 1);
    assert(web_persist_field(ticket, 1) == 1);
    assert(web_persist_field(ticket, 2) == 3);
    assert(web_persist_field(ticket, 3) == 5000);

    int consumed = web_persist_complete(ticket, 1, 1, 1, 3, 5000);
    assert(consumed == 1);
    int kind, outcome; Player p2; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p2, &level) == 1);
    assert(kind == 2);
    assert(outcome == 1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_save_echo_mismatch(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t ticket = web_persist_begin_save(&p, 1);
    int consumed = web_persist_complete(ticket, 1, 1, 1, 3, 9999);
    assert(consumed == 1);
    int kind, outcome; Player p2; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p2, &level) == 1);
    assert(outcome == -1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_save_error(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t ticket = web_persist_begin_save(&p, 1);
    int consumed = web_persist_complete(ticket, -1, 0, 0, 0, 0);
    assert(consumed == 1);
    int kind, outcome; Player p2; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p2, &level) == 1);
    assert(outcome == -1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_save_cancelled(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t ticket = web_persist_begin_save(&p, 1);
    web_persist_cancel();
    assert(web_persist_kind() == 0);
    assert(web_persist_ticket() == 0);
    int kind, outcome; Player p2; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p2, &level) == 1);
    assert(outcome == -3);
    assert(web_persist_complete(ticket, 1, 1, 1, 3, 5000) == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_two_loads_sequential(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t t1 = web_persist_begin_load();
    web_persist_complete(t1, 0, 0, 0, 0, 0);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    int32_t t2 = web_persist_begin_load();
    assert(t2 > 0);
    assert(t2 != t1);
    web_persist_complete(t2, 1, 1, 0, 3, 1000);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_load_then_save(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t t1 = web_persist_begin_load();
    web_persist_complete(t1, 0, 0, 0, 0, 0);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    Player p = make_player(3, 5000);
    int32_t t2 = web_persist_begin_save(&p, 1);
    assert(t2 > 0);
    assert(t2 != t1);
    web_persist_complete(t2, 1, 1, 1, 3, 5000);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_save_then_load(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t t1 = web_persist_begin_save(&p, 1);
    web_persist_complete(t1, 1, 1, 1, 3, 5000);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    int32_t t2 = web_persist_begin_load();
    assert(t2 > 0);
    assert(t2 != t1);
    web_persist_complete(t2, 1, 1, 0, 3, 1000);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_late_load_response_against_new_save(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t t_load = web_persist_begin_load();
    assert(t_load > 0);
    assert(web_persist_complete(t_load, 0, 0, 0, 0, 0) == 1);
    assert(web_persist_bridge_take_result(NULL, NULL, NULL, NULL) == 1);
    Player p = make_player(3, 5000);
    int32_t t_save = web_persist_begin_save(&p, 1);
    assert(t_save > 0);
    assert(t_save != t_load);
    assert(web_persist_complete(t_load, 0, 0, 0, 0, 0) == 0);   /* resposta tardia */
    assert(web_persist_complete(t_save, 1, 1, 1, 3, 5000) == 1);
    assert(web_persist_bridge_take_result(NULL, NULL, NULL, NULL) == 1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_late_save_response_against_new_load(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t t_save = web_persist_begin_save(&p, 1);
    assert(t_save > 0);
    assert(web_persist_complete(t_save, 1, 1, 1, 3, 5000) == 1);
    assert(web_persist_bridge_take_result(NULL, NULL, NULL, NULL) == 1);
    int32_t t_load = web_persist_begin_load();
    assert(t_load > 0);
    assert(t_load != t_save);
    assert(web_persist_complete(t_save, 1, 1, 1, 3, 5000) == 0); /* resposta tardia */
    assert(web_persist_complete(t_load, 0, 0, 0, 0, 0) == 1);
    assert(web_persist_bridge_take_result(NULL, NULL, NULL, NULL) == 1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_cancelled_ticket_never_reused(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t t1 = web_persist_begin_load();
    web_persist_cancel();
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    assert(web_persist_complete(t1, 1, 1, 0, 3, 1000) == 0);
    int32_t t2 = web_persist_begin_load();
    assert(t2 > t1); /* contador monotónico */
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_outcome_rejection(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_load();
    int consumed = web_persist_complete(ticket, 7, 1, 0, 3, 1000);
    assert(consumed == 1);
    int kind, outcome; Player p; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -1);

    ticket = web_persist_begin_load();
    consumed = web_persist_complete(ticket, -5, 1, 0, 3, 1000);
    assert(consumed == 1);
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -1);

    ticket = web_persist_begin_load();
    consumed = web_persist_complete(ticket, INT32_MIN, 1, 0, 3, 1000);
    assert(consumed == 1);
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -1);

    ticket = web_persist_begin_load();
    consumed = web_persist_complete(ticket, INT32_MAX, 1, 0, 3, 1000);
    assert(consumed == 1);
    assert(web_persist_bridge_take_result(&kind, &outcome, &p, &level) == 1);
    assert(outcome == -1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_ticket_exhaustion(void)
{
    TEST_START();
    web_persist_bridge_init();
#ifdef WEB_PERSIST_TEST
    web_persist_bridge_test_set_counter(INT32_MAX - 2);
    Player p = make_player(3, 5000);
    int32_t t1 = web_persist_begin_save(&p, 1);
    assert(t1 == INT32_MAX - 1);
    web_persist_complete(t1, 1, 1, 1, 3, 5000);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    int32_t t2 = web_persist_begin_save(&p, 1);
    assert(t2 == INT32_MAX);
    web_persist_complete(t2, 1, 1, 1, 3, 5000);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    /* Contador esgotado */
    assert(web_persist_begin_save(&p, 1) == 0);
    assert(web_persist_begin_load() == 0);
    /* cleanup NÃO repõe contador */
    web_persist_bridge_cleanup();
    assert(web_persist_begin_load() == 0);
    web_persist_bridge_test_set_counter(0);
#endif
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_score_boundaries(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, -1);
    assert(web_persist_begin_save(&p, 1) == 0);

    p.pontuacao = 2147473648L;
    assert(web_persist_begin_save(&p, 1) == 0);

    p.pontuacao = INT32_MAX;
    assert(web_persist_begin_save(&p, 1) == 0);

    p.pontuacao = WEB_CHECKPOINT_SCORE_MAX;
    int32_t ticket = web_persist_begin_save(&p, 1);
    assert(ticket > 0);
    assert(web_persist_field(ticket, 3) == WEB_CHECKPOINT_SCORE_MAX);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_state_preserved_on_failure(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t ticket = web_persist_begin_save(&p, 1);
    web_persist_complete(ticket, -1, 0, 0, 0, 0);
    int kind, outcome; Player p2; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p2, &level) == 1);
    assert(outcome == -1);
    int32_t t2 = web_persist_begin_save(&p, 1);
    assert(t2 > 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_concurrent_load_save_blocked(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t t1 = web_persist_begin_load();
    assert(t1 > 0);
    int32_t t2 = web_persist_begin_save(&p, 1);
    assert(t2 == 0);
    web_persist_complete(t1, 0, 0, 0, 0, 0);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    t2 = web_persist_begin_save(&p, 1);
    assert(t2 > 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_take_result_only_fills_on_load_ok(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t t1 = web_persist_begin_save(&p, 1);
    web_persist_complete(t1, 1, 1, 1, 3, 5000);
    Player p_out; int level; int kind, outcome;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p_out, &level) == 1);
    assert(kind == 2);
    assert(outcome == 1);
    /* player/level não preenchidos para SAVE */

    int32_t t2 = web_persist_begin_load();
    web_persist_complete(t2, 0, 0, 0, 0, 0);
    assert(web_persist_bridge_take_result(&kind, &outcome, &p_out, &level) == 1);
    assert(outcome == 0);

    int32_t t3 = web_persist_begin_load();
    web_persist_complete(t3, 1, 1, 0, 3, 1000);
    assert(web_persist_bridge_take_result(&kind, &outcome, &p_out, &level) == 1);
    assert(outcome == 1);
    assert(p_out.vidas == 3);
    assert(p_out.pontuacao == 1000);
    assert(level == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

int main(void)
{
    test_init_cleanup();
    test_null_pointers();
    test_load_ok();
    test_load_not_found();
    test_load_error();
    test_load_cancelled();
    test_load_invalid_snapshot();
    test_load_unavailable_level3();
    test_save_ok();
    test_save_echo_mismatch();
    test_save_error();
    test_save_cancelled();
    test_two_loads_sequential();
    test_load_then_save();
    test_save_then_load();
    test_late_load_response_against_new_save();
    test_late_save_response_against_new_load();
    test_cancelled_ticket_never_reused();
    test_outcome_rejection();
    test_ticket_exhaustion();
    test_score_boundaries();
    test_state_preserved_on_failure();
    test_concurrent_load_save_blocked();
    test_take_result_only_fills_on_load_ok();
    printf("\nAll %d tests passed.\n", g_test_count);
    return 0;
}
