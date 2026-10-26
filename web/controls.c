#include "controls.h"
#include "space.h"
static int *const bindings[5] = {
    &tecla_up, &tecla_down, &tecla_left, &tecla_right, &tecla_fire
};
static int capturing = -1, error;
int web_control_key(int action)
{
    return action >= 0 && action < 5 ? *bindings[action] : -1;
}
int web_control_set(int action, int key)
{
    if (action < 0 || action >= 5 || key < 32 || key > 126) return -1;
    if (key >= 'A' && key <= 'Z') key += 'a' - 'A';
    if (key == 'q' || key == 'n') return -2;
    for (int i = 0; i < 5; ++i)
        if (i != action && *bindings[i] == key) return -3;
    *bindings[action] = key;
    return 0;
}
void web_capture_begin(int action)
{
    capturing = action >= 0 && action < 5 ? action : -1;
    error = 0;
}
int web_capture_active(void) { return capturing >= 0; }
int web_capture_error(void) { return error; }
void web_capture_key(int key)
{
    if (capturing < 0) return;
    if (key == 27) { capturing = -1; error = 0; return; }
    error = web_control_set(capturing, key);
    if (!error) capturing = -1;
}
