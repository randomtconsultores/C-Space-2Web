#ifndef WEB_INTRO_H
#define WEB_INTRO_H
void web_intro_begin(double now_ms);
/* Returns 1 only once the final frame's 40 ms has elapsed, or when inactive. */
int web_intro_update(double now_ms);
#endif
