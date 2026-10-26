#ifndef WEB_CONTROLS_H
#define WEB_CONTROLS_H
/* Actions 0..4: up, down, left, right, fire. Values are printable ASCII. */
int web_control_key(int action);
int web_control_set(int action, int key);
void web_capture_begin(int action);
int web_capture_active(void);
int web_capture_error(void);
void web_capture_key(int key);
#endif
