#include "runtime.h"
#include <math.h>
#include <string.h>
static unsigned char held[6];
static unsigned long sequence[4], serial;
static int fire_pending, enter_pending, keys[2], count, cursor;
static double last_ms, accumulated;
static int clock_started;
void web_flush_pressed(void)
{
    fire_pending = enter_pending = count = cursor = 0;
}
void web_reset(void)
{
    memset(held, 0, sizeof held);
    memset(sequence, 0, sizeof sequence);
    serial = 0;
    web_flush_pressed();
    clock_started = 0;
    last_ms = accumulated = 0;
}
void web_input(int key, int down)
{
    if (key < 0 || key >= 6) return;
    down = !!down;
    if (down && !held[key]) {
        if (key < 4) sequence[key] = ++serial;
        if (key == 4) fire_pending = 1;
        if (key == 5) enter_pending = 1;
    }
    held[key] = (unsigned char)down;
}
int web_enter_pressed(void)
{
    int result = enter_pending;
    enter_pending = 0;
    return result;
}
void web_begin_tick(void)
{
    static const int movement[4] = {'w', 's', 'a', 'd'};
    count = cursor = 0;
    if (fire_pending) keys[count++] = ' ';
    fire_pending = 0;
    int selected = -1;
    for (int i = 0; i < 4; ++i)
        if (held[i] && (selected < 0 || sequence[i] > sequence[selected])) selected = i;
    if (selected >= 0) keys[count++] = movement[selected];
}
int web_getch(void) { return cursor < count ? keys[cursor++] : -1; }
int web_steps(double now_ms, double step_ms)
{
    if (!isfinite(now_ms) || !isfinite(step_ms) || step_ms <= 0) {
        clock_started = 0;
        accumulated = 0;
        return 0;
    }
    if (!clock_started) {
        last_ms = now_ms;
        clock_started = 1;
        return 0;
    }
    double delta = now_ms - last_ms;
    last_ms = now_ms;
    if (delta < 0 || delta > 250) {
        accumulated = 0;
        return 0;
    }
    accumulated += delta;
    int updates = 0;
    while (accumulated >= step_ms && updates < 5) {
        accumulated -= step_ms;
        ++updates;
    }
    if (accumulated >= step_ms) accumulated = fmod(accumulated, step_ms);
    return updates;
}
