#include "runtime.h"
#include "terminal.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void)
{
    web_reset();
    web_input(-1, 1); web_input(WEB_KEY_COUNT, 1);
    web_input(0, 1); web_input(4, 1); web_input(4, 1);
    web_begin_tick();
    assert(web_getch() == ' ' && web_getch() == 'w' && web_getch() == -1);
    web_begin_tick(); assert(web_getch() == 'w' && web_getch() == -1);
    web_input(1, 1); web_begin_tick(); assert(web_getch() == 's');
    web_input(1, 0); web_begin_tick(); assert(web_getch() == 'w');
    web_input(4, 0); web_input(4, 1); web_begin_tick();
    assert(web_getch() == ' ' && web_getch() == 'w' && web_getch() == -1);
    web_input(5, 1); assert(web_enter_pressed() == 1 && web_enter_pressed() == 0);
    web_reset();
    web_input(0, 1); web_input(4, 1); web_input(9, 1);
    web_begin_tick(); assert(web_getch() == 'q' && web_getch() == -1);
    web_begin_tick(); assert(web_getch() == 'q' && web_getch() == -1);
    web_input(9, 0); web_begin_tick();
    assert(web_getch() == 'w' && web_getch() == -1);
    web_input(9, 1); web_input(9, 0);
    web_begin_tick(); assert(web_getch() == 'q' && web_getch() == -1);
    assert(web_take_press(6) == 0);
    web_reset(); web_input(6, 1); web_input(6, 0);
    assert(web_take_press(6) == 1 && web_take_press(6) == 0);
    web_begin_tick(); assert(web_getch() == -1);
    web_input(9, 1); web_reset();
    web_begin_tick(); assert(web_getch() == -1);
    assert(web_take_press(10) == 0);
    web_reset();
    web_input(10, 1);
    assert(web_skip_pressed() == 1 && web_skip_pressed() == 0);
    web_input(10, 1); assert(web_skip_pressed() == 0);
    web_input(10, 0); web_input(10, 1); web_input(10, 0);
    assert(web_skip_pressed() == 1 && web_skip_pressed() == 0);
    web_begin_tick(); assert(web_getch() == -1);
    web_input(10, 1); web_flush_pressed(); assert(web_skip_pressed() == 0);
    web_input(10, 0); web_input(10, 1); web_reset(); assert(web_skip_pressed() == 0);
    web_input(0, 1); web_input(4, 1); web_input(10, 1);
    assert(web_skip_pressed() == 1);
    web_flush_pressed(); web_begin_tick();
    assert(web_getch() == 'w' && web_getch() == -1);
    web_reset();
    web_input(9, 1); web_input(10, 1); web_input(10, 0);
    assert(web_skip_pressed() == 0);
    web_begin_tick(); assert(web_getch() == 'q' && web_getch() == -1);
    web_input(9, 0); assert(web_skip_pressed() == 0);
    web_reset();
    web_input(9, 1); web_input(9, 0); web_input(10, 1); web_input(10, 0);
    assert(web_skip_pressed() == 0);
    web_begin_tick(); assert(web_getch() == 'q' && web_getch() == -1);
    assert(web_skip_pressed() == 0);
    web_reset();
    assert(web_take_press(WEB_KEY_COUNT) == 0);
    assert(web_steps(0, 30) == 0 && web_steps(16, 30) == 0);
    assert(web_steps(32, 30) == 1 && web_steps(90, 30) == 2);
    assert(web_steps(330, 30) == 5 && web_steps(360, 30) == 1);
    assert(web_steps(1360, 30) == 0 && web_steps(1390, 30) == 1);
    assert(web_steps(100, 30) == 0 && web_steps(130, 30) == 1);
    assert(web_steps(NAN, 30) == 0 && web_steps(1, 30) == 0);
    assert(web_steps(2, 0) == 0);
    for (int n = 0; n < 3; ++n) {
        double step = 30 - n * 3;
        web_reset(); assert(web_steps(0, step) == 0);
        int total = 0;
        for (int i = 1; i <= 300; ++i) total += web_steps(i * step / 2, step);
        assert(total == 150);
    }
    web_clear();
    mvprintw(1, 1, "%s %d", "HP", 150);
    assert(web_buffer()[WEB_STRIDE + 1] == 'H');
    mvprintw(2, 1, "<\xe2\x96\x88>");
    assert(web_buffer()[2 * WEB_STRIDE + 1] == '<');
    assert(web_buffer()[2 * WEB_STRIDE + 2] == '#');
    assert(web_buffer()[2 * WEB_STRIDE + 3] == '>');
    assert(mvprintw(-1, 0, "x") == ERR);
    puts("PASS: N edges/reset/Q priority, Q tap/hold, input, fixed timestep and formatting");
    return 0;
}
