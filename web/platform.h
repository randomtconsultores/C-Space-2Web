#ifndef WEB_PLATFORM_H
#define WEB_PLATFORM_H
#include <stdlib.h>
#include "terminal.h"
#include "audio.h"
/* Browser-only compatibility boundary: known legacy requests become bounded IDs.
   No shell execution; the browser consumes queued IDs through Howler.js. */
#define system(command) web_audio_system(command)
#endif
