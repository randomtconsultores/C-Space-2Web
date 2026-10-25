#include "framebuffer.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    web_clear();
    const unsigned char *p = web_buffer();
    assert(web_width() == 40 && web_height() == 15);
    assert(web_buffer_size() == 615 && p[615] == 0);
    for (int y = 0; y < WEB_HEIGHT; ++y) {
        for (int x = 0; x < WEB_WIDTH; ++x)
            assert(p[y * WEB_STRIDE + x] == ' ');
        assert(p[y * WEB_STRIDE + WEB_WIDTH] == '\n');
    }
    assert(web_put(0, 0, 'A') && web_put(39, 14, 'Z'));
    assert(p[0] == 'A' && p[14 * WEB_STRIDE + 39] == 'Z');
    unsigned char saved[WEB_HEIGHT * WEB_STRIDE + 1];
    memcpy(saved, p, sizeof saved);
    assert(!web_put(-1, 0, 'x') && !web_put(40, 0, 'x'));
    assert(!web_put(0, -1, 'x') && !web_put(0, 15, 'x'));
    assert(!web_put(INT_MIN, INT_MAX, 'x'));
    assert(memcmp(saved, p, sizeof saved) == 0);
    assert(web_put(1, 1, 0) && p[WEB_STRIDE + 1] == '?');
    web_clear();
    assert(p[0] == ' ' && p[14 * WEB_STRIDE + 39] == ' ');
    puts("PASS: framebuffer dimensions, clearing, clipping and ASCII");
    return 0;
}
