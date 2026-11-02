#include "persist_bridge.h"
#include "checkpoint.h"
#include <stdint.h>
#include <string.h>

#define WEB_SCORE_MAX 9
#define WEB_SCORE_NAME 8

typedef enum {
    PB_IDLE = 0,
    PB_LOAD_PENDING,
    PB_SAVE_PENDING,
    PB_SCORE_PENDING,
    PB_RESULT_READY
} PersistBridgeState;

static PersistBridgeState s_state = PB_IDLE;
static int32_t s_ticket = 0;
static int32_t s_kind = 0;
static int32_t s_outcome = 0;
static WebCheckpoint s_snapshot;
static Player s_restored_player;
static int s_restored_level;
static int32_t s_ticket_counter = 0;
static int s_bridge_available = 0;

/* Ranking recebido (kind 4). */
static int s_score_count = 0;
static int s_score_position[WEB_SCORE_MAX];
static long s_score_value[WEB_SCORE_MAX];
static char s_score_name[WEB_SCORE_MAX][WEB_SCORE_NAME];

static int32_t next_ticket(void)
{
    if (s_ticket_counter == INT32_MAX) return 0;
    return ++s_ticket_counter;
}

void web_persist_bridge_init(void)
{
    s_state = PB_IDLE;
    s_ticket = 0;
    s_kind = 0;
    s_outcome = 0;
    s_ticket_counter = 0;
    memset(&s_snapshot, 0, sizeof(s_snapshot));
    memset(&s_restored_player, 0, sizeof(s_restored_player));
    s_restored_level = 0;
    memset(s_score_position, 0, sizeof(s_score_position));
    memset(s_score_value, 0, sizeof(s_score_value));
    memset(s_score_name, 0, sizeof(s_score_name));
    s_score_count = 0;
}

void web_persist_bridge_cleanup(void)
{
    s_state = PB_IDLE;
    s_ticket = 0;
    s_kind = 0;
    s_outcome = 0;
    /* s_ticket_counter NÃO é reposto */
    /* s_bridge_available NÃO é reposto: sobrevive a init/cleanup */
    memset(&s_snapshot, 0, sizeof(s_snapshot));
    memset(&s_restored_player, 0, sizeof(s_restored_player));
    s_restored_level = 0;
    memset(s_score_position, 0, sizeof(s_score_position));
    memset(s_score_value, 0, sizeof(s_score_value));
    memset(s_score_name, 0, sizeof(s_score_name));
    s_score_count = 0;
}

void web_persist_bridge_set_available(int available)
{
    s_bridge_available = available ? 1 : 0;
}

int web_persist_bridge_available(void)
{
    return s_bridge_available;
}

int32_t web_persist_kind(void)
{
    if (s_state == PB_LOAD_PENDING) return 1;
    if (s_state == PB_SAVE_PENDING) return 2;
    if (s_state == PB_SCORE_PENDING) return 4;
    return 0;
}

int32_t web_persist_ticket(void)
{
    if (s_state == PB_LOAD_PENDING || s_state == PB_SAVE_PENDING ||
        s_state == PB_SCORE_PENDING) return s_ticket;
    return 0;
}

int web_persist_result_ready(void)
{
    return s_state == PB_RESULT_READY ? 1 : 0;
}

static int score_ticket_ok(int32_t ticket)
{
    return ticket > 0 && ticket == s_ticket && s_kind == 4 &&
           (s_state == PB_SCORE_PENDING || s_state == PB_RESULT_READY);
}

int32_t web_persist_score_count(int32_t ticket)
{
    if (!score_ticket_ok(ticket)) return -1;
    return s_score_count;
}

int32_t web_persist_score_position(int32_t ticket, int32_t index)
{
    if (!score_ticket_ok(ticket)) return -1;
    if (index < 0 || index >= s_score_count) return -1;
    return s_score_position[index];
}

int32_t web_persist_score_score(int32_t ticket, int32_t index)
{
    if (!score_ticket_ok(ticket)) return -1;
    if (index < 0 || index >= s_score_count) return -1;
    {
        long v = s_score_value[index];
        if (v < 0 || v > 2147473647L) return -1;
        return (int32_t)v;
    }
}

int32_t web_persist_score_name_char(int32_t ticket, int32_t index,
                                    int32_t char_index)
{
    if (!score_ticket_ok(ticket)) return -1;
    if (index < 0 || index >= s_score_count) return -1;
    if (char_index < 0 || char_index >= WEB_SCORE_NAME) return -1;
    return (int32_t)(unsigned char)s_score_name[index][char_index];
}

int32_t web_persist_score_feed_entry(int32_t ticket, int32_t index,
                                     int32_t position, int32_t score)
{
    if (ticket <= 0 || ticket != s_ticket || s_kind != 4 ||
        s_state != PB_SCORE_PENDING) return 0;
    if (index < 0 || index >= WEB_SCORE_MAX) return 0;
    if (position < 1) return 0;
    if (score < 0 || score > 2147473647) return 0;
    s_score_position[index] = (int)position;
    s_score_value[index] = (long)score;
    s_score_name[index][0] = '\0';
    if (index + 1 > s_score_count) s_score_count = index + 1;
    return 1;
}

int32_t web_persist_score_feed_name_char(int32_t ticket, int32_t index,
                                         int32_t char_index, int32_t char_code)
{
    if (ticket <= 0 || ticket != s_ticket || s_kind != 4 ||
        s_state != PB_SCORE_PENDING) return 0;
    if (index < 0 || index >= WEB_SCORE_MAX) return 0;
    if (char_index < 0 || char_index >= WEB_SCORE_NAME) return 0;
    if (char_code < 0 || char_code > 255) return 0;
    s_score_name[index][char_index] = (char)(unsigned char)char_code;
    if (char_index + 1 > s_score_count && char_index < WEB_SCORE_NAME - 1) {
        /* keep count consistent with fed entries; name termination handled by caller */
    }
    return 1;
}

int32_t web_persist_field(int32_t ticket, int32_t field)
{
    if (ticket != s_ticket || ticket <= 0) return -1;
    if (s_kind != 2) return -1;
    if (s_state != PB_SAVE_PENDING) return -1;

    switch (field) {
        case 0: return s_snapshot.format_version;
        case 1: return s_snapshot.level_index;
        case 2: return s_snapshot.lives;
        case 3: {
            long score = s_snapshot.score;
            if (score < 0 || score > WEB_CHECKPOINT_SCORE_MAX) return -1;
            return (int32_t)score;
        }
        default: return -1;
    }
}

static int validate_load_outcome(int32_t outcome)
{
    return outcome == 1 || outcome == 0 || outcome == -1 ||
           outcome == -2 || outcome == -3 || outcome == -4;
}

static int validate_save_outcome(int32_t outcome)
{
    return outcome == 1 || outcome <= -1;
}

int32_t web_persist_complete(int32_t ticket, int32_t outcome,
                             int32_t format_version, int32_t level_index,
                             int32_t lives, int32_t score)
{
    if (ticket != s_ticket || ticket <= 0) return 0;
    if (s_state != PB_LOAD_PENDING && s_state != PB_SAVE_PENDING &&
        s_state != PB_SCORE_PENDING) return 0;

    if (s_kind == 1) { /* LOAD */
        if (!validate_load_outcome(outcome)) outcome = -1;
        s_outcome = outcome;

        if (outcome == 1) {
            WebCheckpoint cp = {
                .format_version = format_version,
                .level_index = level_index,
                .lives = lives,
                .score = (long)score
            };
            if (web_checkpoint_validate(&cp) == WEB_CHECKPOINT_OK) {
                Player restored;
                int restored_level;
                int res = web_checkpoint_restore(&cp, &restored, &restored_level);
                if (res == WEB_CHECKPOINT_OK) {
                    s_restored_player = restored;
                    s_restored_level = restored_level;
                    s_snapshot = cp;
                    s_outcome = 1;
                    s_state = PB_RESULT_READY;
                    return 1;
                } else if (res == WEB_CHECKPOINT_UNAVAILABLE) {
                    s_outcome = -4;
                } else {
                    s_outcome = -2;
                }
            } else {
                s_outcome = -2;
            }
        }
        s_state = PB_RESULT_READY;
        return 1;

    } else if (s_kind == 2) { /* SAVE */
        if (!validate_save_outcome(outcome)) outcome = -1;
        if (outcome == 1 &&
            format_version == s_snapshot.format_version &&
            level_index == s_snapshot.level_index &&
            lives == s_snapshot.lives &&
            score == (int32_t)s_snapshot.score) {
            s_outcome = 1;
        } else {
            s_outcome = -1;
        }
        s_state = PB_RESULT_READY;
        return 1;
    } else if (s_kind == 4) { /* SCORE_LIST */
        if (outcome != 1 && outcome != 0 && outcome > -1) outcome = -1;
        s_outcome = outcome;
        s_state = PB_RESULT_READY;
        return 1;
    }
    return 0;
}

void web_persist_cancel(void)
{
    if (s_state == PB_LOAD_PENDING || s_state == PB_SAVE_PENDING ||
        s_state == PB_SCORE_PENDING) {
        s_outcome = -3;
        s_state = PB_RESULT_READY;
    }
}

int32_t web_persist_begin_load(void)
{
    if (s_state != PB_IDLE) return 0;
    int32_t ticket = next_ticket();
    if (ticket <= 0) return 0;
    s_state = PB_LOAD_PENDING;
    s_ticket = ticket;
    s_kind = 1;
    s_outcome = 0;
    return ticket;
}

int32_t web_persist_begin_save(const Player *player, int next_level)
{
    if (s_state != PB_IDLE) return 0;
    WebCheckpoint cp;
    if (web_checkpoint_capture(player, next_level, &cp) != WEB_CHECKPOINT_OK) return 0;
    int32_t ticket = next_ticket();
    if (ticket <= 0) return 0;
    s_snapshot = cp;
    s_state = PB_SAVE_PENDING;
    s_ticket = ticket;
    s_kind = 2;
    s_outcome = 0;
    return ticket;
}

int32_t web_persist_begin_score_list(void)
{
    if (s_state != PB_IDLE) return 0;
    int32_t ticket = next_ticket();
    if (ticket <= 0) return 0;
    s_state = PB_SCORE_PENDING;
    s_ticket = ticket;
    s_kind = 4;
    s_outcome = 0;
    s_score_count = 0;
    return ticket;
}

int web_persist_bridge_take_result(int *out_kind, int *out_outcome,
                                   Player *out_player, int *out_level)
{
    if (s_state != PB_RESULT_READY) return 0;

    if (out_kind) *out_kind = s_kind;
    if (out_outcome) *out_outcome = s_outcome;

    if (s_outcome == 1 && s_kind == 1) {
        if (out_player) *out_player = s_restored_player;
        if (out_level) *out_level = s_restored_level;
    }

    s_state = PB_IDLE;
    s_ticket = 0;
    s_kind = 0;
    s_outcome = 0;
    return 1;
}

#ifdef WEB_PERSIST_TEST
void web_persist_bridge_test_set_counter(int32_t value)
{
    if (value >= 0 && value <= INT32_MAX) s_ticket_counter = value;
}
#endif
