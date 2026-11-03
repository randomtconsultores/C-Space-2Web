#include "intro.h"
#include "space.h"
#include "terminal.h"
#include <math.h>
#include <stdlib.h>
static int columns_y[LARGURA_TELA];
static int next_frame, active;
static double last_ms, accumulated;
/* Same column update and title overlay as animacao_intro_binaria in draw.c. */
static void draw_frame(int frame)
{
    for (int x = 1; x < LARGURA_TELA - 1; ++x) {
        if (columns_y[x] - 1 >= 0 && columns_y[x] - 1 < ALTURA_TELA)
            mvaddch(columns_y[x] - 1, x, ' ');
        if (columns_y[x] >= 0 && columns_y[x] < ALTURA_TELA)
            mvaddch(columns_y[x], x, rand() % 2 == 0 ? '0' : '1');
        ++columns_y[x];
        if (columns_y[x] >= ALTURA_TELA) columns_y[x] = 0;
    }
    if (frame > 35) {
        const int cx = LARGURA_TELA / 2, cy = ALTURA_TELA / 2;
        for (int y = cy - 2; y <= cy + 2; ++y)
            for (int x = cx - 12; x <= cx + 12; ++x) mvaddch(y, x, ' ');
        mvprintw(cy - 2, cx - 11, "======================");
        mvprintw(cy - 1, cx - 10, " C-SPACE BOOT SEQUENCE ");
        mvprintw(cy, cx - 9, "  SYSTEMS: [ONLINE]   ");
        mvprintw(cy + 2, cx - 11, "======================");
    }
}
void web_intro_begin(double now_ms)
{
    for (int x = 0; x < LARGURA_TELA; ++x) columns_y[x] = -(rand() % ALTURA_TELA);
    web_clear();
    last_ms = isfinite(now_ms) ? now_ms : 0;
    accumulated = 0;
    active = 1;
    draw_frame(0);
    next_frame = 1;
}
int web_intro_update(double now_ms)
{
    if (!active) return 1;
    if (!isfinite(now_ms)) return 0;
    double delta = now_ms - last_ms;
    last_ms = now_ms;
    if (delta < 0 || delta > 250) {
        accumulated = 0;
        return 0;
    }
    accumulated += delta;
    int updates = 0;
    while (accumulated >= 40 && updates < 5) {
        accumulated -= 40;
        if (next_frame == 80) {
            active = 0;
            web_clear();
            return 1;
        }
        draw_frame(next_frame++);
        ++updates;
        if (next_frame == 80) {
            /* The final frame must actually be presented before its hold starts. */
            accumulated = 0;
            return 0;
        }
    }
    if (accumulated >= 40) accumulated = fmod(accumulated, 40);
    return 0;
}
