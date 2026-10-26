#include "ui.h"
#include "menu.h"
#include "terminal.h"
enum MenuPage { MAIN_PAGE, CONTROLS_PAGE, CREDITS_PAGE, NOTICE_PAGE };
static enum MenuPage page;
static int selection;
void web_menu_open(void)
{
    page = MAIN_PAGE;
    selection = OPCAO_PLAY;
}
enum WebMenuAction web_menu_update(int up, int down, int enter, int back)
{
    if (page != MAIN_PAGE) {
        if (enter || back) page = MAIN_PAGE;
        return WEB_MENU_NONE;
    }
    if (up && selection > OPCAO_PLAY) --selection;
    if (down && selection < OPCAO_SAIR) ++selection;
    if (!enter) return WEB_MENU_NONE;
    switch (selection) {
    case OPCAO_PLAY: return WEB_MENU_PLAY;
    case OPCAO_SAIR: return WEB_MENU_EXIT;
    case OPCAO_COMANDOS: page = CONTROLS_PAGE; break;
    case OPCAO_CREDITOS: page = CREDITS_PAGE; break;
    default: page = NOTICE_PAGE; break;
    }
    return WEB_MENU_NONE;
}
void web_menu_render(void)
{
    web_clear();
    const int cx = LARGURA_TELA / 2, cy = ALTURA_TELA / 2;
    if (page == MAIN_PAGE) {
        static const char *labels[] = {
            "PLAY", "LOAD GAME", "CONTROLS", "CREDITS", "HIGH SCORES", "EXIT"
        };
        mvprintw(cy - 5, cx - 7, "SPACE GAME C");
        mvprintw(cy - 4, cx - 10, "====================");
        for (int i = OPCAO_PLAY; i <= OPCAO_SAIR; ++i)
            mvprintw(cy - 2 + i, cx - 8, "%s %s", i == selection ? "-> " : "   ", labels[i]);
        mvprintw(13, 2, "W/S or arrows: select  ENTER: open");
    } else if (page == CREDITS_PAGE) {
        mvprintw(cy - 1, cx - 12, "Developed in IEFP Course");
        mvprintw(cy, cx - 6, "Final Lab C");
        mvprintw(cy + 2, 3, "ENTER / Q: return to menu");
    } else if (page == CONTROLS_PAGE) {
        mvprintw(2, 7, "SETTINGS & CONTROLS");
        mvprintw(4, 3, "W / S: move up / down");
        mvprintw(5, 3, "A / D: move left / right (stage 2+)");
        mvprintw(6, 3, "SPACE: fire once per press");
        mvprintw(8, 3, "Sound: OFF (not migrated yet)");
        mvprintw(9, 3, "Key remapping: pending migration");
        mvprintw(12, 3, "ENTER / Q: return to menu");
    } else {
        mvprintw(4, 4, "%s", selection == OPCAO_LOAD ? "LOAD GAME" : "HIGH SCORES");
        mvprintw(6, 2, "Persistence not migrated yet.");
        mvprintw(8, 2, "No save or ranking data is read.");
        mvprintw(11, 3, "ENTER / Q: return to menu");
    }
}
