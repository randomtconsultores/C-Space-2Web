#include "ui.h"
#include "runtime.h"
#include "framebuffer.h"
#include "audio.h"
#include "space.h"
#include "persist_bridge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void select_down(int n)
{
    for (int i = 0; i < n; ++i)
        assert(web_menu_update(0, 1, 0, 0) == WEB_MENU_NONE);
}
static void test_ranking_apply_ok(void);
static void test_ranking_apply_empty(void);
static void test_ranking_apply_error(void);
static void test_submit_message(void);
int main(void)
{
    web_menu_open();
    web_menu_render();
    assert(strstr((const char *)web_buffer(), "SPACE GAME C"));
    web_menu_update(1, 0, 0, 0);
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_PLAY);
    web_menu_open(); select_down(20);
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_EXIT);
    web_menu_open(); select_down(1);
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_LOAD);
    web_menu_render();
    assert(strstr((const char *)web_buffer(), "SPACE GAME C"));
    assert(!strstr((const char *)web_buffer(), "Persistence not migrated yet."));
    assert(web_menu_update(0, 0, 0, 1) == WEB_MENU_NONE);
    web_menu_update(1, 0, 0, 0);
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_PLAY);
    web_menu_open(); select_down(2); web_menu_update(0, 0, 1, 0);
    web_menu_render();
    assert(strstr((const char *)web_buffer(), "MOVE UP: [w]"));
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_NONE);
    web_menu_update(0, 0, 0, 1);
    web_menu_render(); assert(strstr((const char *)web_buffer(), "SPACE GAME C"));
    web_menu_open(); select_down(3); web_menu_update(0, 0, 1, 0);
    web_menu_render(); assert(strstr((const char *)web_buffer(), "Developed in IEFP Course"));
    web_menu_open(); select_down(4); web_menu_update(0, 0, 1, 0);
    web_menu_render(); assert(strstr((const char *)web_buffer(), "HIGH SCORES"));
    web_menu_open(); select_down(2); web_menu_update(0, 0, 1, 0);
    som_ativo = 0;
    web_menu_render(); assert(strstr((const char *)web_buffer(), "SOUND: [OFF]"));
    assert(web_sound_enabled() == 0);
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_NONE);
    assert(web_sound_enabled() == 1);
    assert(web_audio_system("play -q -n synth 0.20 sine 600:1800 vol 0.35 > /dev/null 2>&1 &") == 0);
    web_menu_render(); assert(strstr((const char *)web_buffer(), "SOUND: [ON]"));
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_NONE);
    assert(web_sound_enabled() == 0);
    assert(web_audio_take() == WEB_AUDIO_NONE);
    web_reset();
    web_input(7, 1); web_input(7, 1);
    assert(web_take_press(7) == 1 && web_take_press(7) == 0);
    web_begin_tick(); assert(web_getch() == -1);
    web_input(6, 1); assert(web_take_press(6) == 1);
    web_begin_tick(); assert(web_getch() == -1);
    assert(web_take_press(-1) == 0 && web_take_press(9) == 0);
    web_input(8, 1); web_reset(); assert(web_take_press(8) == 0);

    web_ranking_begin();
    web_ranking_render();
    assert(strstr((const char *)web_buffer(), "LOADING..."));
    assert(strstr((const char *)web_buffer(), "ESC: cancela"));

    int pos[2] = {1, 2};
    long sc[2] = {1000, 500};
    char name[2][WEB_SCORE_NAME] = {{'A','B',0}, {'Z',0}};
    web_ranking_set(2, pos, sc, (const char *)name);
    web_ranking_render();
    assert(strstr((const char *)web_buffer(), "GALACTIC RANKING"));
    assert(strstr((const char *)web_buffer(), "AB"));
    assert(strstr((const char *)web_buffer(), "NO SCORES YET") == NULL);

    web_ranking_set(0, NULL, NULL, NULL);
    web_ranking_render();
    assert(strstr((const char *)web_buffer(), "NO SCORES YET"));

    web_ranking_error();
    web_ranking_render();
    assert(strstr((const char *)web_buffer(), "Erro ao carregar ranking."));

    test_ranking_apply_ok();
    test_ranking_apply_empty();
    test_ranking_apply_error();
    test_submit_message();

    puts("PASS: menu bounds, actions, SOUND toggle/queue clear and UI-only key edges");
    return 0;
}

static void test_ranking_apply_ok(void)
{
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_score_list();
    assert(ticket > 0);
    assert(web_persist_score_feed_entry(ticket, 0, 1, 1000) == 1);
    assert(web_persist_score_feed_name_char(ticket, 0, 0, 'A') == 1);
    assert(web_persist_score_feed_name_char(ticket, 0, 1, 0) == 1);
    assert(web_persist_score_feed_entry(ticket, 1, 2, 500) == 1);
    assert(web_persist_score_feed_name_char(ticket, 1, 0, 'Z') == 1);
    assert(web_persist_score_feed_name_char(ticket, 1, 1, 0) == 1);
    assert(web_persist_complete(ticket, 1, 0, 0, 0, 0) == 1);
    web_ranking_apply_bridge_result(ticket, 4, 1);
    web_ranking_render();
    assert(strstr((const char *)web_buffer(), "GALACTIC RANKING"));
    assert(strstr((const char *)web_buffer(), "1. A       : 0001000"));
    assert(strstr((const char *)web_buffer(), "2. Z       : 0000500"));
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    web_persist_bridge_cleanup();
}

static void test_ranking_apply_empty(void)
{
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_score_list();
    assert(ticket > 0);
    assert(web_persist_complete(ticket, 1, 0, 0, 0, 0) == 1);
    web_ranking_apply_bridge_result(ticket, 4, 1);
    web_ranking_render();
    assert(strstr((const char *)web_buffer(), "GALACTIC RANKING"));
    assert(strstr((const char *)web_buffer(), "NO SCORES YET"));
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    web_persist_bridge_cleanup();
}

static void test_ranking_apply_error(void)
{
    web_persist_bridge_init();
    int32_t ticket = web_persist_begin_score_list();
    assert(ticket > 0);
    assert(web_persist_complete(ticket, -1, 0, 0, 0, 0) == 1);
    web_ranking_apply_bridge_result(ticket, 4, -1);
    web_ranking_render();
    assert(strstr((const char *)web_buffer(), "Erro ao carregar ranking."));
    web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
    web_persist_bridge_cleanup();
}

static void test_submit_message(void)
{
    assert(strcmp(web_submit_message(1, 1), "NEW HIGH SCORE") == 0);
    assert(strcmp(web_submit_message(1, 9), "NEW HIGH SCORE") == 0);
    assert(strcmp(web_submit_message(1, 10), "SCORE SAVED") == 0);
    assert(strcmp(web_submit_message(1, 100), "SCORE SAVED") == 0);
    assert(strcmp(web_submit_message(0, 0), "Could not save score.") == 0);
    assert(strcmp(web_submit_message(0, 5), "Could not save score.") == 0);
}
