#ifndef WEB_TRANSITION_H
#define WEB_TRANSITION_H
/* completed_level is zero-based; the original final level has no transition. */
void web_transition_begin(int completed_level, double now_ms);
/* Returns 1 when the complete sequence has finished, or when inactive. */
int web_transition_update(double now_ms);
#endif
