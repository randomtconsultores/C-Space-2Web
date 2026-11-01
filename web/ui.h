#ifndef WEB_UI_H
#define WEB_UI_H
enum WebMenuAction { WEB_MENU_NONE, WEB_MENU_PLAY, WEB_MENU_EXIT, WEB_MENU_LOAD };
void web_menu_open(void);
enum WebMenuAction web_menu_update(int up, int down, int enter, int back);
void web_menu_render(void);
int web_sound_enabled(void);
void web_set_sound(int enabled);
#endif
