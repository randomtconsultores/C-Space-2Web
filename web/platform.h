#ifndef WEB_PLATFORM_H
#define WEB_PLATFORM_H
#include <stdlib.h>
#include "terminal.h"
/* Browser build only: SoX shell commands cannot provide browser audio. */
static inline int web_no_native_sound(const char *command)
{
    (void)command;
    return -1;
}
#define system(command) web_no_native_sound(command)
#endif
