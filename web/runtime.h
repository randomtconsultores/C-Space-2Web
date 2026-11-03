#ifndef WEB_RUNTIME_H
#define WEB_RUNTIME_H
/* Key indices: W,S,A,D,Space,Enter,Escape,ArrowUp,ArrowDown,Q,N. */
#define WEB_KEY_COUNT 11
void web_input(int key, int down);
void web_reset(void);
void web_flush_pressed(void);
int web_enter_pressed(void);
int web_take_press(int key);
/* Consume one N press: reach the boss arena, not the next level. Q has priority. */
int web_skip_pressed(void);
void web_begin_tick(void);
int web_getch(void);
int web_steps(double now_ms, double step_ms);
#endif
