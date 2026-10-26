#include "levels.h"
#include "boss.h"
#include "draw.h"
#include "framebuffer.h"
#include "runtime.h"
#include "ui.h"
#include "intro.h"
#include <emscripten.h>
#include <string.h>
_Static_assert(LARGURA_TELA == WEB_WIDTH, "Framebuffer width mismatch");
_Static_assert(ALTURA_TELA == WEB_HEIGHT, "Framebuffer height mismatch");
enum Phase { MENU, CLOSED, INTRO, PLAYING, STAGE_CLEAR, GAME_OVER, VICTORY, INVALID_LEVEL };
static enum Phase phase;
static char mapa[ALTURA_TELA][LARGURA_MAPA];
static Projétil enemy_shots[MAX_TIROS_INIMIGO];
static Player player;
static Boss boss;
static int level, offset, scroll_ticks;
static const unsigned seeds[MAX_NIVEIS] = {12345, 98765, 42424, 86420};
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
static void show_menu(void)
{
    phase = MENU;
    web_menu_open();
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
                break;
            }
        }
    }
    player.proximo_patamar_pontos += 10000;
}
static void tick(void)
{
    web_begin_tick();
    process_input(&player, mapa, offset, level >= 1);
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
        if (boss.ativo == 0) boss.ativo = 1;
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
                } else if (player.imunidade <= 0 && (cell == '#' || cell == 'O' || cell == 'x')) {
                    take_damage(1);
                }
            }
        }
    }
}
static void render(void)
{
    if (phase == INTRO) return; /* Intro updates its own persistent framebuffer. */
    if (phase == MENU) { web_menu_render(); return; }
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
        return;
    }
    if (phase == VICTORY) {
        web_clear();
        mvprintw(7, 4, "TOTAL VICTORY! ENTER: MENU");
        return;
    }
    draw_game(&player, mapa, offset, level, &boss, enemy_shots);
    if (phase == STAGE_CLEAR) mvprintw(7, 2, "STAGE CLEAR - ENTER: NEXT");
    if (phase == GAME_OVER) {
        mvprintw(6, 4, "YOU WERE CRUSHED!");
        mvprintw(7, 4, "FINAL SCORE: %ld", player.pontuacao);
        mvprintw(9, 4, "ENTER: MAIN MENU");
    }
}
static void frame(void)
{
    double now = emscripten_get_now();
    int enter = web_enter_pressed();
    if (phase == MENU) {
        int up = web_take_press(0) | web_take_press(7);
        int down = web_take_press(1) | web_take_press(8);
        enum WebMenuAction action = web_menu_update(up, down, enter, web_take_press(6));
        if (action == WEB_MENU_PLAY) {
            web_reset();
            web_intro_begin(now);
            phase = INTRO;
        }
        else if (action == WEB_MENU_EXIT) { phase = CLOSED; web_reset(); }
    } else if (phase == INTRO) {
        if (web_intro_update(now)) new_game();
    } else if (enter && phase != PLAYING) {
        if (phase == STAGE_CLEAR) {
            if (++level >= MAX_NIVEIS) phase = VICTORY;
            else begin_level();
        } else show_menu();
    }
    double step = (DELAY - level * 3000) / 1000.0;
    int updates = web_steps(now, step);
    for (int i = 0; i < updates && phase == PLAYING; ++i) tick();
    render();
    present();
}
int main(void)
{
    som_ativo = 0;
    init_game();
    show_menu();
    emscripten_set_main_loop(frame, 0, 0);
    return 0;
}
