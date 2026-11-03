#include "controls.h"
#include "ui.h"
#include "runtime.h"
#include "framebuffer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void down(int count)
{
    for (int i = 0; i < count; ++i) web_menu_update(0, 1, 0, 0);
}
static void enter(void) { web_menu_update(0, 0, 1, 0); }
int main(void)
{
    assert(web_control_key(-1) == -1 && web_control_key(5) == -1);
    assert(web_control_key(0) == 'w' && web_control_key(4) == ' ');
    assert(web_control_set(-1, 'k') == -1 && web_control_set(5, 'k') == -1);
    assert(web_control_set(0, 31) == -1 && web_control_set(0, 127) == -1);
    assert(web_control_set(0, 'Q') == -2 && web_control_set(0, 'n') == -2);
    assert(web_control_set(0, 's') == -3 && web_control_key(0) == 'w');
    web_capture_key('k'); assert(web_control_key(0) == 'w');
    web_menu_open(); down(2); enter(); down(1); enter();
    assert(web_capture_active());
    web_capture_key('a'); assert(web_capture_error() == -3 && web_capture_active());
    web_menu_render(); assert(strstr((const char *)web_buffer(), "already assigned"));
    web_capture_key('q'); assert(web_capture_error() == -2 && web_capture_active());
    web_capture_key('K'); assert(!web_capture_active() && web_control_key(0) == 'k');
    web_menu_update(0, 0, 0, 0); web_menu_render();
    assert(strstr((const char *)web_buffer(), "MOVE UP: [k]"));
    web_input(0, 1); web_begin_tick(); assert(web_getch() == 'k');
    web_reset();
    enter(); assert(web_capture_active());
    web_capture_key(27); web_menu_update(0, 0, 0, 0);
    assert(!web_capture_active() && web_control_key(0) == 'k');
    down(4); enter(); web_capture_key('h'); web_menu_update(0, 0, 0, 0);
    assert(web_control_key(4) == 'h');
    web_input(4, 1); web_begin_tick(); assert(web_getch() == 'h');
    web_reset();
    web_menu_update(0, 0, 0, 1); web_menu_render();
    assert(strstr((const char *)web_buffer(), "SPACE GAME C"));
    assert(web_control_key(0) == 'k' && web_control_key(4) == 'h');
    assert(web_control_set(0, 'w') == 0 && web_control_set(4, ' ') == 0);
    web_capture_begin(99); assert(!web_capture_active());
    puts("PASS: remapping, validation, cancel, UI capture and native key emission");
    return 0;
}
