#include "transition.h"
#include "space.h"
#include "terminal.h"
#include <math.h>
static int completed, step_index, active;
static double last_ms, remaining_ms;
static int upgrade_steps(void) { return completed == 0 ? 6 : 0; }
static double duration(void)
{
    if (step_index < 4) return 150;
    if (step_index == 4) return 2000;
    int index = step_index - 5;
    if (index < upgrade_steps()) return index % 2 == 0 ? 800 : 200;
    index -= upgrade_steps();
    return index % 2 == 0 ? 700 : 300;
}
static void draw_step(void)
{
    web_clear();
    if (step_index < 4) {
        int radius = step_index + 1;
        for (int x = 0; x < LARGURA_TELA; ++x) {
            mvaddch(0, x, '#');
            mvaddch(ALTURA_TELA - 1, x, '#');
        }
        mvaddch(7 - radius, 34 - radius, '*');
        mvaddch(7 - radius, 34 + radius, '+');
        mvaddch(7 + radius, 34 - radius, '.');
        mvaddch(7 + radius, 34 + radius, '*');
        if (radius > 2) {
            mvaddch(7, 34 - radius * 2, '<');
            mvaddch(7, 34 + radius * 2, '>');
        }
        return;
    }
    if (step_index == 4) return;
    int index = step_index - 5;
    int upgrade = index < upgrade_steps();
    if (!upgrade) index -= upgrade_steps();
    if (index % 2 != 0) return;
    const int cx = LARGURA_TELA / 2, cy = ALTURA_TELA / 2;
    for (int x = 0; x < LARGURA_TELA; ++x) {
        mvaddch(cy - 3, x, upgrade ? '=' : '-');
        mvaddch(cy + 3, x, upgrade ? '=' : '-');
    }
    if (upgrade) {
        mvprintw(cy - 1, cx - 8, "UPGRADE UNLOCKED!");
        mvprintw(cy + 1, cx - 12, "USE A / D TO MOVE < & >");
    } else {
        mvprintw(cy - 1, cx - 9, "STAGE %d COMPLETED!", completed + 1);
        mvprintw(cy + 1, cx - 11, "Preparing next zone...");
    }
}
void web_transition_begin(int completed_level, double now_ms)
{
    active = completed_level >= 0 && completed_level < MAX_NIVEIS - 1;
    if (!active) return;
    completed = completed_level;
    step_index = 0;
    last_ms = isfinite(now_ms) ? now_ms : 0;
    remaining_ms = duration();
    draw_step();
}
int web_transition_update(double now_ms)
{
    if (!active) return 1;
    if (!isfinite(now_ms)) return 0;
    double delta = now_ms - last_ms;
    last_ms = now_ms;
    if (delta < 0 || delta > 250) return 0;
    remaining_ms -= delta;
    if (remaining_ms > 0) return 0;
    if (++step_index == 9 + upgrade_steps()) {
        active = 0;
        web_clear();
        return 1;
    }
    /* At most one step per callback: each newly presented step gets its full hold. */
    remaining_ms = duration();
    draw_step();
    return 0;
}
