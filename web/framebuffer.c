#include "framebuffer.h"
#include <string.h>
static unsigned char buffer[WEB_HEIGHT * WEB_STRIDE + 1];
void web_clear(void)
{
    for (int y = 0; y < WEB_HEIGHT; ++y) {
        memset(buffer + y * WEB_STRIDE, ' ', WEB_WIDTH);
        buffer[y * WEB_STRIDE + WEB_WIDTH] = '\n';
    }
    buffer[WEB_HEIGHT * WEB_STRIDE] = 0;
}
int web_put(int x, int y, unsigned char ch)
{
    if (x < 0 || x >= WEB_WIDTH || y < 0 || y >= WEB_HEIGHT)
        return 0;
    buffer[y * WEB_STRIDE + x] = (ch >= 32 && ch <= 126) ? ch : '?';
    return 1;
}
const unsigned char *web_buffer(void) { return buffer; }
int web_buffer_size(void) { return WEB_HEIGHT * WEB_STRIDE; }
int web_width(void) { return WEB_WIDTH; }
int web_height(void) { return WEB_HEIGHT; }
