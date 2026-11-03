#include "transition.h"
#include "space.h"
#include "framebuffer.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int contains(const char *text)
{
    return strstr((const char *)web_buffer(), text) != NULL;
}
static void blank(void)
{
    const unsigned char *p = web_buffer();
    for (int y = 0; y < WEB_HEIGHT; ++y) {
        for (int x = 0; x < WEB_WIDTH; ++x) assert(p[y * WEB_STRIDE + x] == ' ');
        assert(p[y * WEB_STRIDE + WEB_WIDTH] == '\n');
    }
    assert(p[WEB_HEIGHT * WEB_STRIDE] == 0);
}
static int advance(double *now, double target)
{
    int result = 0;
    while (*now < target) {
        *now += target - *now > 50 ? 50 : target - *now;
        result = web_transition_update(*now);
        if (*now < target) assert(result == 0);
    }
    return result;
}
int main(void)
{
    assert(web_transition_update(0) == 1);
    double now = 0;
    srand(77); int expected_random = rand(); srand(77);
    web_transition_begin(0, now);
    assert(web_buffer()[6 * WEB_STRIDE + 33] == '*');
    assert(advance(&now, 149) == 0);
    assert(web_buffer()[6 * WEB_STRIDE + 33] == '*');
    assert(advance(&now, 150) == 0);
    assert(web_buffer()[5 * WEB_STRIDE + 32] == '*');
    assert(advance(&now, 450) == 0);
    assert(web_buffer()[3 * WEB_STRIDE + 30] == '*');
    assert(advance(&now, 600) == 0); blank();
    assert(advance(&now, 2599) == 0); blank();
    assert(advance(&now, 2600) == 0 && contains("UPGRADE UNLOCKED!"));
    assert(advance(&now, 3400) == 0); blank();
    assert(advance(&now, 3600) == 0 && contains("UPGRADE UNLOCKED!"));
    assert(advance(&now, 4400) == 0); blank();
    assert(advance(&now, 4600) == 0 && contains("UPGRADE UNLOCKED!"));
    assert(advance(&now, 5400) == 0); blank();
    assert(advance(&now, 5600) == 0 && contains("STAGE 1 COMPLETED!"));
    assert(advance(&now, 6300) == 0); blank();
    assert(advance(&now, 6600) == 0 && contains("Preparing next zone..."));
    assert(advance(&now, 7300) == 0); blank();
    assert(advance(&now, 7599) == 0);
    assert(advance(&now, 7600) == 1); blank();
    assert(rand() == expected_random);
    for (int level = 1; level < MAX_NIVEIS - 1; ++level) {
        now = 0; web_transition_begin(level, now);
        assert(advance(&now, 2600) == 0);
        char title[32]; snprintf(title, sizeof title, "STAGE %d COMPLETED!", level + 1);
        assert(contains(title) && !contains("UPGRADE UNLOCKED!"));
        assert(advance(&now, 4599) == 0);
        assert(advance(&now, 4600) == 1); blank();
    }
    web_transition_begin(1, 0);
    unsigned char initial[WEB_HEIGHT * WEB_STRIDE + 1];
    memcpy(initial, web_buffer(), sizeof initial);
    assert(web_transition_update(NAN) == 0);
    assert(web_transition_update(10000) == 0);
    assert(memcmp(initial, web_buffer(), sizeof initial) == 0);
    assert(web_transition_update(9900) == 0);
    assert(memcmp(initial, web_buffer(), sizeof initial) == 0);
    web_transition_begin(1, 0);
    assert(web_transition_update(250) == 0);
    assert(web_buffer()[5 * WEB_STRIDE + 32] == '*');
    assert(web_transition_update(399) == 0);
    assert(web_buffer()[5 * WEB_STRIDE + 32] == '*');
    assert(web_transition_update(400) == 0);
    assert(web_buffer()[4 * WEB_STRIDE + 31] == '*');
    web_transition_begin(1, 0);
    assert(memcmp(initial, web_buffer(), sizeof initial) == 0);
    web_transition_begin(MAX_NIVEIS - 1, 0); assert(web_transition_update(0) == 1);
    web_transition_begin(-1, 0); assert(web_transition_update(0) == 1);
    puts("PASS: transition phases, 7600/4600 ms timing, restart, suspension, bounds and RNG isolation");
    return 0;
}
