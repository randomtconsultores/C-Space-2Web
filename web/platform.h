#ifndef WEB_PLATFORM_H
#define WEB_PLATFORM_H
#include <stdlib.h>
#include "terminal.h"
#include "audio.h"
/* Browser-only compatibility boundary: known SoX requests become bounded IDs.
   No shell execution; actual browser audio is a separate, still-pending step. */
#define system(command) web_audio_system(command)
#endif
