#include "intro.h"
#include "framebuffer.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int has_title(void)
{
    return strstr((const char *)web_buffer(), "C-SPACE BOOT SEQUENCE") != NULL;
}
static void assert_blank(void)
{
    const unsigned char *p = web_buffer();
    for (int y = 0; y < WEB_HEIGHT; ++y) {
        for (int x = 0; x < WEB_WIDTH; ++x) assert(p[y * WEB_STRIDE + x] == ' ');
        assert(p[y * WEB_STRIDE + WEB_WIDTH] == '\n');
    }
    assert(p[WEB_HEIGHT * WEB_STRIDE] == 0);
}
int main(void)
{
    assert(web_intro_update(0) == 1);
    srand(123);
    web_intro_begin(0);
    assert(!has_title());
    for (int i = 1; i <= 79; ++i) {
        assert(web_intro_update(i * 40.0) == 0);
        assert(has_title() == (i > 35));
    }
    assert(web_intro_update(3199) == 0 && has_title());
    assert(web_intro_update(3200) == 1);
    assert_blank();
    assert(web_intro_update(3240) == 1);
    srand(123); web_intro_begin(0);
    for (int i = 1; i <= 78; ++i) assert(web_intro_update(i * 40.0) == 0);
    assert(web_intro_update(3200) == 0 && has_title());
    assert(web_intro_update(3239) == 0 && has_title());
    assert(web_intro_update(3240) == 1);
    assert_blank();
    srand(123); web_intro_begin(0);
    unsigned char initial[WEB_HEIGHT * WEB_STRIDE + 1];
    memcpy(initial, web_buffer(), sizeof initial);
    assert(web_intro_update(NAN) == 0);
    assert(web_intro_update(10000) == 0);
    assert(memcmp(initial, web_buffer(), sizeof initial) == 0);
    assert(web_intro_update(10040) == 0);
    assert(web_intro_update(9000) == 0);
    srand(123); web_intro_begin(0);
    assert(web_intro_update(240) == 0);
    for (int t = 280; t <= 1440; t += 40) assert(web_intro_update(t) == 0);
    assert(!has_title()); /* At most five updates were consumed for the 240 ms gap. */
    assert(web_intro_update(1480) == 0 && has_title());
    srand(123); web_intro_begin(0);
    assert(memcmp(initial, web_buffer(), sizeof initial) == 0);
    puts("PASS: intro 80-frame duration, title timing, restart, suspended deltas and catch-up cap");
    return 0;
}
