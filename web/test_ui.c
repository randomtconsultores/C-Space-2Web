#include "ui.h"
#include "runtime.h"
#include "framebuffer.h"
#include "audio.h"
#include "space.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void select_down(int n)
{
    for (int i = 0; i < n; ++i)
        assert(web_menu_update(0, 1, 0, 0) == WEB_MENU_NONE);
}
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
    assert(web_menu_update(0, 0, 1, 0) == WEB_MENU_NONE);
    web_menu_render();
    assert(strstr((const char *)web_buffer(), "Persistence not migrated yet."));
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
    puts("PASS: menu bounds, actions, SOUND toggle/queue clear and UI-only key edges");
    return 0;
}
