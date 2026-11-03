#include "audio.h"
#include <stddef.h>
#include <string.h>
_Static_assert(WEB_AUDIO_EVENT_COUNT <= 256, "Audio IDs must fit one byte");
/* Exact legacy literals: no shell, parser, allocation, timing or gameplay RNG. */
static const char *const commands[WEB_AUDIO_EVENT_COUNT] = {
    [WEB_AUDIO_PICKUP] = "play -q -n synth 0.20 sine 600:1800 vol 0.35 > /dev/null 2>&1 &",
    [WEB_AUDIO_LASER_DAMAGE] = "play -q -n synth 0.35 brownnoise synth 0.35 sine 120 vol 0.40 > /dev/null 2>&1 &",
    [WEB_AUDIO_BOSS_HIT] = "play -q -n synth 0.03 sine 1500 vol 0.20 > /dev/null 2>&1 &",
    [WEB_AUDIO_CORE_HIT] = "play -q -n synth 0.03 sine 1800 vol 0.25 > /dev/null 2>&1 &",
    [WEB_AUDIO_BOSS_EXPLOSION] = "play -q -n synth 0.80 brownnoise synth 0.80 sine 300:40 vol 0.45 > /dev/null 2>&1 &",
    [WEB_AUDIO_ROCK_BREAK] = "play -q -n synth 0.08 sine 180:60 vol 0.35 > /dev/null 2>&1 &",
    [WEB_AUDIO_ENEMY_EXPLOSION] = "play -q -n synth 0.15 sine 350:50 vol 0.40 > /dev/null 2>&1 &",
    [WEB_AUDIO_BOSS_LOCK] = "play -q -n synth 0.15 sine 300:900 vol 0.35 > /dev/null 2>&1 &",
    [WEB_AUDIO_BOSS_PHASE] = "play -q -n synth 0.50 sine 100:600 vol 0.4 > /dev/null 2>&1 &",
    [WEB_AUDIO_LASER_AIM] = "play -q -n synth 0.04 sine 2000 vol 0.25 > /dev/null 2>&1 &",
    [WEB_AUDIO_MEGA_LASER] = "play -q -n synth 0.60 square 150 vol 0.35 > /dev/null 2>&1 &",
    [WEB_AUDIO_BOSS_CHARGE] = "play -q -n synth 0.20 sine 400:1200 vol 0.35 > /dev/null 2>&1 &",
    [WEB_AUDIO_PLAYER_DAMAGE] = "play -q -n synth 0.06 sine 220:110 synth 0.06 sine 220:110 vol 0.4 > /dev/null 2>&1 &",
    [WEB_AUDIO_REWARD_SPAWN] = "play -q -n synth 0.05 square 800 vol 0.15 > /dev/null 2>&1 &",
    [WEB_AUDIO_BOSS_ENTRY] = "play -q -n synth 0.12 square 440 : synth 0.12 square 440 : synth 0.12 square 440 : synth 0.35 square 280 vol 0.30 > /dev/null 2>&1 &",
    [WEB_AUDIO_BODY_DAMAGE] = "play -q -n synth 0.08 sine 200:150 synth 0.08 sine 200:150 vol 0.4 > /dev/null 2>&1 &",
};
static unsigned char events[WEB_AUDIO_CAPACITY];
static unsigned head, count;
int web_audio_system(const char *command)
{
    if (command == NULL) return -1;
    for (int id = 1; id < WEB_AUDIO_EVENT_COUNT; ++id) {
        if (commands[id] != NULL && strcmp(command, commands[id]) == 0) {
            if (count == WEB_AUDIO_CAPACITY) return -1; /* Drop newest, not oldest. */
            events[(head + count) % WEB_AUDIO_CAPACITY] = (unsigned char)id;
            ++count;
            return 0;
        }
    }
    return -1;
}
int web_audio_take(void)
{
    if (count == 0) return WEB_AUDIO_NONE;
    int event = events[head];
    head = (head + 1) % WEB_AUDIO_CAPACITY;
    --count;
    return event;
}
void web_audio_clear(void)
{
    head = count = 0;
}
