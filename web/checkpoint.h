#ifndef WEB_CHECKPOINT_H
#define WEB_CHECKPOINT_H
#include "space.h"
#define WEB_CHECKPOINT_VERSION 1
#define WEB_CHECKPOINT_LEVELS 4
#define WEB_CHECKPOINT_SCORE_MAX 2147473647L

typedef struct {
    int format_version, level_index, lives;
    long score;
} WebCheckpoint;

enum WebCheckpointResult {
    WEB_CHECKPOINT_OK = 1,
    WEB_CHECKPOINT_INVALID = -1,
    WEB_CHECKPOINT_UNAVAILABLE = -2
};

/* Logical DTO, never a raw binary/Wasm/SQLite serialization. */
int web_checkpoint_validate(const WebCheckpoint *checkpoint);
int web_checkpoint_capture(const Player *player, int next_level, WebCheckpoint *output);
/* Validate first. On failure neither output is modified. Config is read only. */
int web_checkpoint_restore(const WebCheckpoint *checkpoint, Player *output, int *level);

/* Local one-shot LOAD tickets, not authentication or server revisions. */
typedef struct { int serial, pending; } WebCheckpointGate;
int web_checkpoint_gate_begin(WebCheckpointGate *gate);
void web_checkpoint_gate_cancel(WebCheckpointGate *gate);
int web_checkpoint_gate_consume(WebCheckpointGate *gate, int ticket);
#endif
