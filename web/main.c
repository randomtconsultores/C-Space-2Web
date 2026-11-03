#include "levels.h"
#include "boss.h"
#include "draw.h"
#include "framebuffer.h"
#include "runtime.h"
#include "ui.h"
#include "intro.h"
#include "transition.h"
#include "persist_bridge.h"
#include <emscripten.h>
#include <string.h>
_Static_assert(LARGURA_TELA == WEB_WIDTH, "Framebuffer width mismatch");
_Static_assert(ALTURA_TELA == WEB_HEIGHT, "Framebuffer height mismatch");
enum Phase { MENU, CLOSED, INTRO, PLAYING, STAGE_CLEAR, TRANSITION, GAME_OVER, VICTORY, INVALID_LEVEL, LOAD_PENDING, LOAD_NOTICE, SAVE_PENDING, RANKING_PENDING, RANKING_NOTICE, NAME_ENTRY, SUBMIT_PENDING, SUBMIT_NOTICE };
static enum Phase phase;
static const char *load_notice;
static const char *save_notice;
static int save_notice_ttl;
static char mapa[ALTURA_TELA][LARGURA_MAPA];
static Projétil enemy_shots[MAX_TIROS_INIMIGO];
static Player player;
static Boss boss;
static int level, offset, scroll_ticks;
static const unsigned seeds[MAX_NIVEIS] = {12345, 98765, 42424, 86420};

/* Estado da leitura do ranking (incremento B). */
static int ranking_ticket;
static int ranking_count;
static int ranking_position[WEB_SCORE_MAX];
static long ranking_score[WEB_SCORE_MAX];
static char ranking_name[WEB_SCORE_MAX][WEB_SCORE_NAME];

/* Estado da escrita do ranking (incremento C). */
static int submit_ticket;
static int submit_ok;
static int submit_position;
static const char *submit_message;
EM_JS(void, present, (), {
    if (Module["spaceRender"]) Module["spaceRender"]();
});
static void clear_shots(void)
{
    memset(player.tiro, 0, sizeof player.tiro);
    memset(enemy_shots, 0, sizeof enemy_shots);
}
static void begin_level(void)
{
    if (level < 0 || level >= MAX_NIVEIS ||
        configuracao_niveis[level].comprimento_mapa < LARGURA_TELA ||
        configuracao_niveis[level].comprimento_mapa > LARGURA_MAPA) {
        phase = INVALID_LEVEL;
        web_reset();
        return;
    }
    player.x = 2;
    player.y = 7;
    player.proximo_patamar_pontos = 10000;
    offset = scroll_ticks = 0;
    memset(&boss, 0, sizeof boss);
    boss.x = LARGURA_TELA - 4;
    boss.y = 7;
    boss.vida = BOSS_VIDA_MAX;
    boss.direcao_y = 1;
    boss.direcao_x = -1;
    reset_boss_ia();
    clear_shots();
    gerar_mapa_dinamico(mapa, seeds[level], level);
    web_reset();
    phase = PLAYING;
}
static void advance_level(void)
{
    if (++level >= MAX_NIVEIS) {
        phase = VICTORY;
        web_reset();
    } else begin_level();
}
static void show_menu(void)
{
    phase = MENU;
    web_menu_open();
    web_reset();
}
static void show_load_notice(const char *message)
{
    load_notice = message;
    phase = LOAD_NOTICE;
    web_reset();
}
static void show_save_notice(const char *message)
{
    save_notice = message;
    save_notice_ttl = 120;
    web_reset();
}
static void handle_load_result(void)
{
    int kind, outcome, restored_level;
    Player restored;
    if (!web_persist_bridge_take_result(&kind, &outcome, &restored, &restored_level)) return;
    if (kind != 1) {
        show_load_notice("Unexpected LOAD result.");
        return;
    }
    if (outcome == 1) {
        long threshold = restored.proximo_patamar_pontos;
        player = restored;
        level = restored_level;
        begin_level();
        player.proximo_patamar_pontos = threshold;
        return;
    }
    switch (outcome) {
    case 0: show_load_notice("No checkpoint saved."); break;
    case -2: show_load_notice("Invalid checkpoint."); break;
    case -3: show_menu(); break;
    case -4: show_load_notice("Checkpoint level unavailable."); break;
    default: show_load_notice("Unable to load checkpoint."); break;
    }
}
static void handle_save_result(double now)
{
    int kind, outcome;
    if (!web_persist_bridge_take_result(&kind, &outcome, NULL, NULL)) return;
    if (kind != 2 || outcome != 1) {
        show_save_notice("Save not confirmed.");
    }
    web_transition_begin(level, now);
    phase = TRANSITION;
}

static void handle_ranking_result(void)
{
    int kind, outcome, i, ci, count;
    int pos[WEB_SCORE_MAX];
    long sc[WEB_SCORE_MAX];
    char names[WEB_SCORE_MAX * WEB_SCORE_NAME];
    if (!web_persist_result_ready()) return;
    if (ranking_ticket <= 0) {
        web_ranking_error();
        phase = RANKING_NOTICE;
        web_reset();
        return;
    }
    /* Leitura dos acessores ANTES do take_result (estado ainda e' RESULT_READY). */
    count = web_persist_score_count(ranking_ticket);
    if (count < 0) count = 0;
    if (count > WEB_SCORE_MAX) count = WEB_SCORE_MAX;
    for (i = 0; i < count; i++) {
        pos[i] = web_persist_score_position(ranking_ticket, i);
        sc[i] = web_persist_score_score(ranking_ticket, i);
        for (ci = 0; ci < WEB_SCORE_NAME; ci++)
            names[i * WEB_SCORE_NAME + ci] = (char)web_persist_score_name_char(ranking_ticket, i, ci);
        names[i * WEB_SCORE_NAME + WEB_SCORE_NAME - 1] = '\0';
    }
    /* Consome o resultado (obtem kind/outcome e reinicia o estado). */
    if (!web_persist_bridge_take_result(&kind, &outcome, NULL, NULL)) {
        web_ranking_error();
        phase = RANKING_NOTICE;
        web_reset();
        return;
    }
    if (kind == 4 && outcome == 1)
        web_ranking_set(count, pos, sc, names);
    else
        web_ranking_error();
    phase = RANKING_NOTICE;
    web_reset();
}

static void handle_submit_result(void)
{
    int kind, outcome, position = 0;
    if (!web_persist_result_ready()) return;
    if (submit_ticket > 0)
        position = web_persist_submit_position(submit_ticket);
    if (web_persist_bridge_take_result(&kind, &outcome, NULL, NULL) != 1) {
        submit_ok = 0;
        submit_position = 0;
        submit_message = web_submit_message(0, 0);
        phase = SUBMIT_NOTICE;
        web_reset();
        return;
    }
    if (kind != 3) { show_menu(); return; }
    submit_ok = (outcome == 1);
    submit_position = position;
    submit_message = web_submit_message(submit_ok, position);
    phase = SUBMIT_NOTICE;
    web_reset();
}

static void new_game(void)
{
    memset(&player, 0, sizeof player);
    player.vidas = 3;
    level = 0;
    begin_level();
}
/* Matches the original respawn; body collisions do not clear projectiles. */
static void take_damage(int reset_projectiles)
{
    if (som_ativo)
        system(reset_projectiles
            ? "play -q -n synth 0.06 sine 220:110 synth 0.06 sine 220:110 vol 0.4 > /dev/null 2>&1 &"
            : "play -q -n synth 0.08 sine 200:150 synth 0.08 sine 200:150 vol 0.4 > /dev/null 2>&1 &");
    --player.vidas;
    if (player.vidas <= 0) {
        phase = GAME_OVER;
        return;
    }
    player.x = 2;
    player.y = 7;
    player.imunidade = 66;
    if (reset_projectiles) clear_shots();
    web_flush_pressed();
}
static void reward(void)
{
    if (player.pontuacao < player.proximo_patamar_pontos) return;
    int x = offset + player.x + 18;
    /* Web boundary safety: original adjacent-row lookup lacks these checks. */
    int rows[3] = {player.y, player.y - 1, player.y + 1};
    if (x >= 0 && x < LARGURA_MAPA) {
        for (int i = 0; i < 3; ++i) {
            int y = rows[i];
            if (y >= 0 && y < ALTURA_TELA && mapa[y][x] == ' ') {
                mapa[y][x] = '+';
                if (som_ativo)
                    system("play -q -n synth 0.05 square 800 vol 0.15 > /dev/null 2>&1 &");
                break;
            }
        }
    }
    player.proximo_patamar_pontos += 10000;
}
static void tick(void)
{
    if (web_skip_pressed()) {
        /* Apply native N before map input can inspect a negative sentinel. */
        player.x = 2;
        offset = configuracao_niveis[level].comprimento_mapa - LARGURA_TELA;
        web_flush_pressed();
    } else {
        web_begin_tick();
        process_input(&player, mapa, offset, level >= 1);
    }
    if (player.x == -99) {
        if (player.imunidade <= 0) take_damage(1);
        else player.x = 2;
        if (phase != PLAYING) return;
    }
    reward();
    update_game(&player, mapa, offset, &boss, enemy_shots, level);
    if (player.vidas <= 0) { phase = GAME_OVER; return; }
    int end = configuracao_niveis[level].comprimento_mapa - LARGURA_TELA;
    if (offset >= end) {
        if (boss.ativo == 0) {
            if (som_ativo)
                system("play -q -n synth 0.12 square 440 : synth 0.12 square 440 : synth 0.12 square 440 : synth 0.35 square 280 vol 0.30 > /dev/null 2>&1 &");
            boss.ativo = 1;
        }
        if (boss.ativo == 1 && player.imunidade <= 0 &&
            player.x >= boss.x && player.x <= boss.x + 2 &&
            player.y >= boss.y - 1 && player.y <= boss.y + 1) {
            take_damage(0);
            if (phase != PLAYING) return;
        }
        if (boss.ativo == 2) { phase = STAGE_CLEAR; return; }
    }
    if (++scroll_ticks >= 6) {
        scroll_ticks = 0;
        if (offset < end) {
            ++offset;
            int x = player.x + offset;
            /* A hit can set x=-99 in update_game; never index a negative cell. */
            if (x >= 0 && x < LARGURA_MAPA && player.y >= 0 && player.y < ALTURA_TELA) {
                char cell = mapa[player.y][x];
                if (cell == '+') {
                    if (player.vidas < MAX_VIDAS_JOGADOR) ++player.vidas;
                    player.pontuacao += 500;
                    mapa[player.y][x] = ' ';
                    if (som_ativo)
                        system("play -q -n synth 0.20 sine 600:1800 vol 0.35 > /dev/null 2>&1 &");
                } else if (player.imunidade <= 0 && (cell == '#' || cell == 'O' || cell == 'x')) {
                    take_damage(1);
                }
            }
        }
    }
}
static void render_name_entry(void);

static void render(void)
{
    if (phase == INTRO || phase == TRANSITION) return; /* Timed sequences own the framebuffer. */
    if (phase == MENU) { web_menu_render(); return; }
    if (phase == RANKING_PENDING || phase == RANKING_NOTICE) {
        web_ranking_render();
        return;
    }
    if (phase == NAME_ENTRY) {
        render_name_entry();
        return;
    }
    if (phase == SUBMIT_PENDING) {
        web_clear();
        mvprintw(ALTURA_TELA / 2 - 2, LARGURA_TELA / 2 - 9, "Subindo pontuacao...");
        mvprintw(ALTURA_TELA / 2, LARGURA_TELA / 2 - 10, "ESC: cancela");
        return;
    }
    if (phase == SUBMIT_NOTICE) {
        web_clear();
        const int cx = LARGURA_TELA / 2, cy = ALTURA_TELA / 2;
        mvprintw(cy - 2, cx - 10, "%s", submit_message ? submit_message : "");
        if (submit_ok) mvprintw(cy, cx - 10, "Position: %d", submit_position);
        mvprintw(cy + 3, cx - 10, "ENTER / ESC: menu");
        return;
    }
    if (phase == LOAD_PENDING) {
        web_clear();
        mvprintw(5, 2, "Loading...");
        mvprintw(7, 2, "ESC: cancel");
        return;
    }
    if (phase == SAVE_PENDING) {
        web_clear();
        mvprintw(5, 2, "Saving...");
        mvprintw(7, 2, "ESC: cancel");
        return;
    }
    if (phase == LOAD_NOTICE) {
        web_clear();
        mvprintw(5, 2, "%s", load_notice ? load_notice : "Unable to load checkpoint.");
        mvprintw(7, 2, "ENTER / ESC: return to menu");
        return;
    }
    if (phase == CLOSED) {
        web_clear();
        mvprintw(6, 4, "SESSION CLOSED");
        mvprintw(8, 4, "ENTER: MAIN MENU");
        return;
    }
    if (phase == INVALID_LEVEL) {
        web_clear();
        mvprintw(5, 2, "LEVEL %d: CONFIGURATION MISSING", level + 1);
        mvprintw(7, 2, "Inherited issue; not a victory.");
        mvprintw(9, 2, "ENTER: MAIN MENU");
        if (save_notice_ttl > 0) {
            mvprintw(11, 2, "%s", save_notice ? save_notice : "Save not confirmed.");
            --save_notice_ttl;
        }
        return;
    }
    if (phase == VICTORY) {
        web_clear();
        mvprintw(7, 4, "TOTAL VICTORY! ENTER: MENU");
        return;
    }
    draw_game(&player, mapa, offset, level, &boss, enemy_shots);
    if (save_notice_ttl > 0) {
        mvprintw(5, 2, "%s", save_notice ? save_notice : "Save not confirmed.");
        --save_notice_ttl;
    }
    if (phase == GAME_OVER) {
        mvprintw(6, 4, "YOU WERE CRUSHED!");
        mvprintw(7, 4, "FINAL SCORE: %ld", player.pontuacao);
        mvprintw(9, 4, "ENTER: MAIN MENU");
    }
}

static void render_name_entry(void)
{
    web_clear();
    const int cx = LARGURA_TELA / 2, cy = ALTURA_TELA / 2;
    int len = web_persist_name_length();
    int i;
    mvprintw(cy - 4, cx - 12, "ENTER YOUR NAME");
    mvprintw(cy - 2, cx - 10, "PONTUACAO: %ld", player.pontuacao);
    mvprintw(cy, cx - 10, "NOME (MAX 7 CAR.): ");
    for (i = 0; i < len; i++) {
        int c = web_persist_name_char(i);
        mvaddch(cy, cx + 10 + i, c >= 0 ? (char)c : '?');
    }
    mvprintw(cy + 3, cx - 12, "ENTER: confirma   ESC: cancela");
}

static void frame(void)
{
    double now = emscripten_get_now();
    int enter = web_enter_pressed();
    int loading_frame = phase == LOAD_PENDING || phase == SAVE_PENDING;
    if (phase == MENU) {
        int up = web_take_press(0) | web_take_press(7);
        int down = web_take_press(1) | web_take_press(8);
        enum WebMenuAction action = web_menu_update(up, down, enter, web_take_press(6));
        if (action == WEB_MENU_PLAY) {
            web_reset();
            web_intro_begin(now);
            phase = INTRO;
        }
        else if (action == WEB_MENU_LOAD) {
            if (web_persist_bridge_available() && web_persist_begin_load() > 0) {
                phase = LOAD_PENDING;
                web_reset();
            } else show_load_notice("Unable to start LOAD.");
        }
        else if (action == WEB_MENU_EXIT) { phase = CLOSED; web_reset(); }
        else if (action == WEB_MENU_HIGHSCORES) {
            phase = RANKING_PENDING;
            web_ranking_begin();
            ranking_ticket = 0;
            if (web_persist_bridge_available() &&
                web_persist_begin_score_list() > 0) {
                ranking_ticket = web_persist_ticket();
                web_reset();
            } else {
                web_ranking_error();
                phase = RANKING_NOTICE;
            }
        }
    } else if (phase == INTRO) {
        if (web_intro_update(now)) new_game();
    } else if (phase == TRANSITION) {
        if (web_transition_update(now)) advance_level();
    } else if (phase == LOAD_PENDING) {
        if (web_take_press(6)) {
            /* Discard even a success that became ready before this frame. */
            web_persist_cancel();
            web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
            show_menu();
        } else handle_load_result();
    } else if (phase == LOAD_NOTICE) {
        if (enter || web_take_press(6)) show_menu();
    } else if (phase == SAVE_PENDING) {
        if (web_take_press(6)) {
            web_persist_cancel();
            web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
            show_save_notice("Save not confirmed.");
            web_transition_begin(level, now);
            phase = TRANSITION;
        } else {
            handle_save_result(now);
        }
    } else if (phase == RANKING_PENDING) {
        if (web_take_press(6)) {
            web_ranking_cancel();
            web_persist_cancel();
            web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
            show_menu();
        } else {
            handle_ranking_result();
        }
    } else if (phase == RANKING_NOTICE) {
        if (enter || web_take_press(6)) {
            web_ranking_cancel();
            show_menu();
        }
    } else if (phase == GAME_OVER || phase == VICTORY) {
        if (enter && player.pontuacao > 0 && web_persist_bridge_available()) {
            phase = NAME_ENTRY;
            web_persist_name_start();
            web_reset();
        } else if (enter || web_take_press(6)) {
            show_menu();
        }
    } else if (phase == NAME_ENTRY) {
        int act = web_persist_name_take_action();
        if (act == 1) {
            phase = SUBMIT_PENDING;
            submit_ticket = web_persist_name_submit(player.pontuacao);
            if (submit_ticket <= 0) {
                show_menu();
            } else {
                web_reset();
            }
        } else if (act == -1) {
            web_persist_name_stop();
            show_menu();
        }
    } else if (phase == SUBMIT_PENDING) {
        if (web_take_press(6)) {
            web_persist_cancel();
            web_persist_bridge_take_result(NULL, NULL, NULL, NULL);
            show_menu();
        } else {
            handle_submit_result();
        }
    } else if (phase == SUBMIT_NOTICE) {
        if (enter || web_take_press(6)) show_menu();
    } else if (enter && phase != PLAYING && phase != STAGE_CLEAR &&
               phase != SAVE_PENDING && phase != RANKING_NOTICE) {
        show_menu();
    }
    double step = (DELAY - level * 3000) / 1000.0;
    int updates = web_steps(now, step);
    /* Present a restored level before advancing its simulation. */
    for (int i = 0; i < updates && phase == PLAYING && !loading_frame; ++i) tick();
    if (phase == STAGE_CLEAR) {
        web_reset();
        if (level < MAX_NIVEIS - 1) {
            int next_level = level + 1;
            int config_ok = configuracao_niveis[next_level].comprimento_mapa >= LARGURA_TELA &&
                            configuracao_niveis[next_level].comprimento_mapa <= LARGURA_MAPA;
            if (config_ok && web_persist_bridge_available() &&
                web_persist_begin_save(&player, next_level) > 0) {
                phase = SAVE_PENDING;
                web_reset();
            } else {
                show_save_notice("Save not confirmed.");
                web_transition_begin(level, now);
                phase = TRANSITION;
            }
        } else advance_level();
    }
    render();
    present();
}
int main(void)
{
    som_ativo = 1;
    init_game();
    web_persist_bridge_init();
    show_menu();
    emscripten_set_main_loop(frame, 0, 0);
    return 0;
}
int web_menu_active(void)
{
    return phase == MENU;
}
