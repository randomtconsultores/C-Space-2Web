#ifndef WEB_UI_H
#define WEB_UI_H
enum WebMenuAction { WEB_MENU_NONE, WEB_MENU_PLAY, WEB_MENU_EXIT, WEB_MENU_LOAD, WEB_MENU_HIGHSCORES };
void web_menu_open(void);
enum WebMenuAction web_menu_update(int up, int down, int enter, int back);
void web_menu_render(void);
int web_sound_enabled(void);
void web_set_sound(int enabled);
/* Ranking (incremento B). */
void web_ranking_begin(void);
void web_ranking_set(int count, const int *pos, const long *sc, const char *name);
void web_ranking_error(void);
void web_ranking_render(void);
void web_ranking_cancel(void);
/* Aplica o resultado da ponte (kind/outcome) lendo os acessores (estado
   PB_RESULT_READY, antes de take_result) e chama web_ranking_set/web_ranking_error. */
void web_ranking_apply_bridge_result(int ticket, int kind, int outcome);
/* Escolhe o texto da mensagem de SUBMIT_NOTICE (OK/erro e posicao). Testavel. */
const char *web_submit_message(int ok, int position);
#endif
