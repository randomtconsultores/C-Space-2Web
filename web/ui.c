#include "ui.h"
#include "menu.h"
#include "terminal.h"
#include "controls.h"
enum MenuPage { MAIN_PAGE, CONTROLS_PAGE, CAPTURE_PAGE, CREDITS_PAGE, NOTICE_PAGE };
static enum MenuPage page;
static int selection, control_selection;
static const char *control_labels[5] = {
    "MOVE UP", "MOVE DOWN", "MOVE LEFT", "MOVE RIGHT", "FIRE"
};
void web_menu_open(void)
{
    page = MAIN_PAGE;
    selection = OPCAO_PLAY;
    control_selection = 0;
    web_capture_key(27);
}
enum WebMenuAction web_menu_update(int up, int down, int enter, int back)
{
    if (page == CAPTURE_PAGE) {
        if (!web_capture_active()) page = CONTROLS_PAGE;
        return WEB_MENU_NONE;
    }
    if (page == CONTROLS_PAGE) {
        if (back) { page = MAIN_PAGE; return WEB_MENU_NONE; }
        if (up && control_selection > 0) --control_selection;
        if (down && control_selection < 5) ++control_selection;
        if (enter && control_selection > 0) {
            web_capture_begin(control_selection - 1);
            web_reset();
            page = CAPTURE_PAGE;
        }
        return WEB_MENU_NONE;
    }
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
    case OPCAO_COMANDOS: page = CONTROLS_PAGE; control_selection = 0; break;
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
        mvprintw(cy + 2, 3, "ENTER / ESC: return to menu");
    } else if (page == CONTROLS_PAGE) {
        mvprintw(2, 7, "SETTINGS & CONTROLS");
        mvprintw(4, 3, "%s SOUND: OFF (not migrated)", control_selection == 0 ? "->" : "  ");
        for (int i = 0; i < 5; ++i) {
            int key = web_control_key(i);
            const char *mark = control_selection == i + 1 ? "->" : "  ";
            if (key == ' ')
                mvprintw(5 + i, 3, "%s %s: [SPACE]", mark, control_labels[i]);
            else
                mvprintw(5 + i, 3, "%s %s: [%c]", mark, control_labels[i], key);
        }
        mvprintw(11, 2, "ENTER: modify key  ESC: main menu");
        mvprintw(13, 2, "Bindings last until page reload.");
    } else if (page == CAPTURE_PAGE) {
        mvprintw(4, 3, "NEW KEY FOR %s", control_labels[control_selection - 1]);
        mvprintw(6, 3, "Press a printable ASCII key.");
        mvprintw(8, 3, "ESC: cancel   Q/N: reserved");
        int error = web_capture_error();
        if (error == -1) mvprintw(11, 2, "Only ASCII 32..126 is supported.");
        if (error == -2) mvprintw(11, 2, "Q/N conflict with native shortcuts.");
        if (error == -3) mvprintw(11, 2, "That key is already assigned.");
    } else {
        mvprintw(4, 4, "%s", selection == OPCAO_LOAD ? "LOAD GAME" : "HIGH SCORES");
        mvprintw(6, 2, "Persistence not migrated yet.");
        mvprintw(8, 2, "No save or ranking data is read.");
        mvprintw(11, 3, "ENTER / ESC: return to menu");
    }
}
