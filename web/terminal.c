#include "terminal.h"
#include <stdarg.h>
#include <stdio.h>
int web_mvprintw(int y, int x, const char *format, ...)
{
    char text[256];
    va_list args;
    va_start(args, format);
    int result = vsnprintf(text, sizeof text, format, args);
    va_end(args);
    if (result < 0) return ERR;
    if (y < 0 || y >= WEB_HEIGHT || x >= WEB_WIDTH) return ERR;
    for (const unsigned char *p = (const unsigned char *)text; *p && x < WEB_WIDTH; ++p) {
        /* One ASCII cell per UTF-8 codepoint; the original block glyph becomes #. */
        if ((*p & 0xc0) == 0x80) continue;
        web_put(x++, y, *p < 128 ? *p : '#');
    }
    return result;
}
