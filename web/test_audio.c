#include "platform.h"
#include <assert.h>
#include <stdio.h>
#ifndef system
#error "The Web platform must intercept system; never invoke the host shell"
#endif
static const char *const known[] = {
    "play -q -n synth 0.20 sine 600:1800 vol 0.35 > /dev/null 2>&1 &",
    "play -q -n synth 0.35 brownnoise synth 0.35 sine 120 vol 0.40 > /dev/null 2>&1 &",
    "play -q -n synth 0.03 sine 1500 vol 0.20 > /dev/null 2>&1 &",
    "play -q -n synth 0.03 sine 1800 vol 0.25 > /dev/null 2>&1 &",
    "play -q -n synth 0.80 brownnoise synth 0.80 sine 300:40 vol 0.45 > /dev/null 2>&1 &",
    "play -q -n synth 0.08 sine 180:60 vol 0.35 > /dev/null 2>&1 &",
    "play -q -n synth 0.15 sine 350:50 vol 0.40 > /dev/null 2>&1 &",
    "play -q -n synth 0.15 sine 300:900 vol 0.35 > /dev/null 2>&1 &",
    "play -q -n synth 0.50 sine 100:600 vol 0.4 > /dev/null 2>&1 &",
    "play -q -n synth 0.04 sine 2000 vol 0.25 > /dev/null 2>&1 &",
    "play -q -n synth 0.60 square 150 vol 0.35 > /dev/null 2>&1 &",
    "play -q -n synth 0.20 sine 400:1200 vol 0.35 > /dev/null 2>&1 &",
    "play -q -n synth 0.06 sine 220:110 synth 0.06 sine 220:110 vol 0.4 > /dev/null 2>&1 &",
    "play -q -n synth 0.05 square 800 vol 0.15 > /dev/null 2>&1 &",
    "play -q -n synth 0.12 square 440 : synth 0.12 square 440 : synth 0.12 square 440 : synth 0.35 square 280 vol 0.30 > /dev/null 2>&1 &",
    "play -q -n synth 0.08 sine 200:150 synth 0.08 sine 200:150 vol 0.4 > /dev/null 2>&1 &",
};
int main(void)
{
    assert(sizeof known / sizeof known[0] == WEB_AUDIO_EVENT_COUNT - 1);
    srand(55); int expected_random = rand(); srand(55);
    assert(web_audio_take() == WEB_AUDIO_NONE);
    assert(system(NULL) == -1);
    assert(system("") == -1);
    assert(system("echo must-not-execute") == -1);
    assert(system("play -q -n synth 9 sine 440") == -1);
    assert(web_audio_take() == WEB_AUDIO_NONE);
    for (int i = 0; i < WEB_AUDIO_EVENT_COUNT - 1; ++i) {
        assert(system(known[i]) == 0);
        assert(web_audio_take() == i + 1);
        assert(web_audio_take() == WEB_AUDIO_NONE);
    }
    for (int i = 0; i < WEB_AUDIO_CAPACITY; ++i) assert(system(known[0]) == 0);
    assert(system(known[1]) == -1);
    for (int i = 0; i < WEB_AUDIO_CAPACITY; ++i) assert(web_audio_take() == WEB_AUDIO_PICKUP);
    assert(web_audio_take() == WEB_AUDIO_NONE);
    for (int i = 0; i < 16; ++i) assert(system(known[i]) == 0);
    for (int i = 0; i < 8; ++i) assert(web_audio_take() == i + 1);
    for (int i = 0; i < 8; ++i) assert(system(known[i]) == 0);
    for (int i = 8; i < 16; ++i) assert(web_audio_take() == i + 1);
    for (int i = 0; i < 8; ++i) assert(web_audio_take() == i + 1);
    for (int i = 0; i < 10000; ++i) {
        assert(system(known[i % 16]) == 0);
        assert(web_audio_take() == i % 16 + 1);
    }
    assert(system(known[5]) == 0);
    assert(system(known[7]) == 0);
    web_audio_clear(); web_audio_clear();
    assert(web_audio_take() == WEB_AUDIO_NONE);
    assert(system(known[3]) == 0);
    assert(web_audio_take() == WEB_AUDIO_CORE_HIT);
    assert(rand() == expected_random);
    puts("PASS: 16 legacy audio mappings, platform interception, FIFO, overflow, wrap, clear and RNG isolation");
    return 0;
}
