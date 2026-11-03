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

static void test_score_list_feed_and_read(void);
static void test_score_list_invalid_accessors(void);
static void test_score_list_cancel(void);
static void test_name_feed_chars_and_backspace(void);
static void test_name_feed_enter_and_esc(void);
static void test_name_feed_charset_and_limit(void);
static void test_begin_score_and_accessors(void);
static void test_name_submit(void);

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

static void test_save_cancel_allows_next_operation(void)
{
    TEST_START();
    web_persist_bridge_init();
    Player p = make_player(3, 5000);
    int32_t ticket = web_persist_begin_save(&p, 1);
    assert(web_persist_kind() == 2);
    web_persist_cancel();
    /* Após cancel + consume, a ponte volta a IDLE: nova operação é possível. */
    assert(web_persist_kind() == 0);
    assert(web_persist_ticket() == 0);
    int kind, outcome; Player p2; int level;
    assert(web_persist_bridge_take_result(&kind, &outcome, &p2, &level) == 1);
    assert(outcome == -3);
    int32_t ticket2 = web_persist_begin_save(&p, 2);
    assert(ticket2 > 0);
    assert(ticket2 != ticket);
    assert(web_persist_kind() == 2);
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

static void test_bridge_available_default(void)
{
    TEST_START();
    web_persist_bridge_init();
    assert(web_persist_bridge_available() == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_bridge_set_available(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_bridge_set_available(1);
    assert(web_persist_bridge_available() == 1);
    web_persist_bridge_set_available(0);
    assert(web_persist_bridge_available() == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_bridge_available_survives_cleanup(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_bridge_set_available(1);
    web_persist_bridge_cleanup();
    assert(web_persist_bridge_available() == 1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_bridge_available_survives_init(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_bridge_set_available(1);
    web_persist_bridge_init();
    assert(web_persist_bridge_available() == 1);
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
    test_save_cancel_allows_next_operation();
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
    test_bridge_available_default();
    test_bridge_set_available();
    test_bridge_available_survives_cleanup();
    test_bridge_available_survives_init();
    test_score_list_feed_and_read();
    test_score_list_invalid_accessors();
    test_score_list_cancel();
    test_name_feed_chars_and_backspace();
    test_name_feed_enter_and_esc();
    test_name_feed_charset_and_limit();
    test_begin_score_and_accessors();
    test_name_submit();
    printf("\nAll %d tests passed.\n", g_test_count);
    return 0;
}

static void test_score_list_feed_and_read(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_score_list();
    assert(ticket > 0);
    assert(web_persist_kind() == 4);
    assert(web_persist_ticket() == ticket);

    assert(web_persist_score_feed_entry(ticket, 0, 1, 1000) == 1);
    assert(web_persist_score_feed_name_char(ticket, 0, 0, 'A') == 1);
    assert(web_persist_score_feed_name_char(ticket, 0, 1, 'B') == 1);
    assert(web_persist_score_feed_name_char(ticket, 0, 2, 0) == 1);

    assert(web_persist_score_feed_entry(ticket, 1, 2, 500) == 1);
    assert(web_persist_score_feed_name_char(ticket, 1, 0, 'Z') == 1);
    assert(web_persist_score_feed_name_char(ticket, 1, 1, 0) == 1);

    assert(web_persist_score_count(ticket) == 2);
    assert(web_persist_score_position(ticket, 0) == 1);
    assert(web_persist_score_score(ticket, 0) == 1000);
    assert(web_persist_score_name_char(ticket, 0, 0) == 'A');
    assert(web_persist_score_name_char(ticket, 0, 1) == 'B');
    assert(web_persist_score_name_char(ticket, 0, 2) == 0);
    assert(web_persist_score_position(ticket, 1) == 2);
    assert(web_persist_score_score(ticket, 1) == 500);
    assert(web_persist_score_name_char(ticket, 1, 0) == 'Z');

    int consumed = web_persist_complete(ticket, 1, 0, 0, 0, 0);
    assert(consumed == 1);
    assert(web_persist_result_ready() == 1);

    int kind, outcome;
    assert(web_persist_bridge_take_result(&kind, &outcome, NULL, NULL) == 1);
    assert(kind == 4);
    assert(outcome == 1);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_score_list_invalid_accessors(void)
{
    TEST_START();
    web_persist_bridge_init();
    assert(web_persist_score_count(1) == -1);
    assert(web_persist_score_position(1, 0) == -1);
    assert(web_persist_score_score(1, 0) == -1);
    assert(web_persist_score_name_char(1, 0, 0) == -1);
    assert(web_persist_score_feed_entry(1, 0, 1, 100) == 0);
    assert(web_persist_score_feed_name_char(1, 0, 0, 'A') == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_score_list_cancel(void)
{
    TEST_START();
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_score_list();
    assert(ticket > 0);
    web_persist_cancel();
    assert(web_persist_result_ready() == 1);
    int kind, outcome;
    assert(web_persist_bridge_take_result(&kind, &outcome, NULL, NULL) == 1);
    assert(kind == 4);
    assert(outcome == -3);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_name_feed_chars_and_backspace(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_name_start();
    assert(web_persist_name_active() == 1);
    assert(web_persist_name_feed('A') == 1);
    assert(web_persist_name_feed('b') == 1); /* minuscula -> maiuscula */
    assert(web_persist_name_length() == 2);
    assert(web_persist_name_char(0) == 'A');
    assert(web_persist_name_char(1) == 'B');
    assert(web_persist_name_feed(8) == 1); /* backspace */
    assert(web_persist_name_length() == 1);
    assert(web_persist_name_char(0) == 'A');
    web_persist_name_stop();
    assert(web_persist_name_active() == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_name_feed_enter_and_esc(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_name_start();
    assert(web_persist_name_feed(13) == 0); /* enter vazio: rejeitado */
    assert(web_persist_name_length() == 0);
    assert(web_persist_name_take_action() == 0);
    assert(web_persist_name_feed('X') == 1);
    assert(web_persist_name_feed(13) == 2); /* enter confirma */
    assert(web_persist_name_take_action() == 1);
    web_persist_name_clear();
    assert(web_persist_name_length() == 0);

    web_persist_name_start();
    assert(web_persist_name_feed('Y') == 1);
    assert(web_persist_name_feed(27) == -1); /* esc cancela */
    assert(web_persist_name_take_action() == -1);
    assert(web_persist_name_length() == 0);
    web_persist_name_stop();
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_name_feed_charset_and_limit(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_name_start();
    /* fora do charset ignorados */
    assert(web_persist_name_feed(' ') == 0);
    assert(web_persist_name_feed('!') == 0);
    assert(web_persist_name_feed(127) == 0);
    assert(web_persist_name_length() == 0);
    /* limite 7 */
    const char *s = "ABCDEFG";
    for (int i = 0; s[i]; i++) assert(web_persist_name_feed(s[i]) == 1);
    assert(web_persist_name_length() == 7);
    assert(web_persist_name_feed('Z') == 0); /* buffer cheio ignorado */
    assert(web_persist_name_length() == 7);
    web_persist_name_stop();
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_begin_score_and_accessors(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_name_start();
    assert(web_persist_name_feed('P') == 1);
    assert(web_persist_name_feed('I') == 1);
    assert(web_persist_name_feed('L') == 1);
    assert(web_persist_name_feed('O') == 1);
    assert(web_persist_name_feed('T') == 1);

    int32_t ticket = web_persist_begin_score(42);
    assert(ticket > 0); /* nome nao vazio: inicia submissao */
    assert(web_persist_complete(ticket, 1, 3, 0, 0, 0) == 1);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL); /* volta a IDLE */
    web_persist_name_clear();

    /* nome vazio nao inicia submissao */
    assert(web_persist_begin_score(42) == 0);
    web_persist_name_start();
    assert(web_persist_name_feed('Q') == 1);

    ticket = web_persist_begin_score(1234);
    assert(ticket > 0);
    assert(web_persist_kind() == 3);
    assert(web_persist_ticket() == ticket);
    assert(web_persist_submit_name_char(ticket, 0) == 'Q');
    assert(web_persist_submit_name_char(ticket, 1) == 0);
    assert(web_persist_submit_score(ticket) == 1234);

    int consumed = web_persist_complete(ticket, 1, 5, 0, 0, 0);
    assert(consumed == 1);
    assert(web_persist_result_ready() == 1);
    assert(web_persist_submit_position(ticket) == 5);

    int kind, outcome;
    assert(web_persist_bridge_take_result(&kind, &outcome, NULL, NULL) == 1);
    assert(kind == 3);
    assert(outcome == 1);
    web_persist_name_stop();
    web_persist_bridge_cleanup();
    TEST_END();
}

static void test_name_submit(void)
{
    TEST_START();
    web_persist_bridge_init();
    web_persist_name_start();
    assert(web_persist_name_feed('A') == 1);
    assert(web_persist_name_feed('B') == 1);
    assert(web_persist_name_feed(13) == 2); /* enter confirma */
    assert(web_persist_name_take_action() == 1);
    int32_t ticket = web_persist_name_submit(1234);
    assert(ticket > 0);
    assert(web_persist_name_active() == 0); /* stop limpa o buffer */
    assert(web_persist_name_length() == 0);
    assert(web_persist_submit_name_char(ticket, 0) == 'A');
    assert(web_persist_submit_name_char(ticket, 1) == 'B');
    assert(web_persist_submit_name_char(ticket, 2) == 0);
    assert(web_persist_submit_score(ticket) == 1234);
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL); /* volta a IDLE */
    web_persist_bridge_cleanup();

    /* nome vazio -> submit devolve 0 e name_active == 0 */
    web_persist_bridge_init();
    web_persist_name_start();
    assert(web_persist_name_submit(42) == 0);
    assert(web_persist_name_active() == 0);
    web_persist_bridge_cleanup();
    TEST_END();
}
