/**
+-------------------------------------------------------------------------------+
|                                        SM                                     |
+-------------------------------------------------------------------------------+
|                                                                               |
|        +---------------------------+           +---------------------------+  |
|        |           BUSY            |       *-->|           IDLE            |  |
|        +---------------------------+           +---------------------------+  |
|        | entry/                    |           | entry/                    |  |
|        | exit/                     |           | exit/                     |  |
|        |        +------------+     |           |        +------------+     |  |
|        |    *-->|  PLAYING   |     |           |    *-->|  CLOSED    |     |  |
|        |        +------------+     |   STOP    |        +------------+     |  |
|        |        | entry/     |     |---------->|        | entry/     |     |  |
|        |        | exit/      |     |           |  PLAY  | exit/      |     |  |
|        |        |            |     |           |  +---->|            |     |  |
|        |        |         +--|     |           |  |     |            |     |  |
|        |        |         |  |     |           |  |     |            |     |  |
|        |        | timer/1s|  |     |           |  |     |            |     |  |
|        |        |         |  |     |           |  |     |            |     |  |
|        |        |         +->|     |           |  |     |            |     |  |
|        |        |            |     |           |  |     |            |     |  |
|        |     +--|            |<-+  |           |  |  +--|            |<-+  |  |
|        |     |  +------------+  |  |           |  |  |  +------------+  |  |  |
|        |     |                  |  |   PLAY    |  |  |                  |  |  |
|        |     |PAUSE             |  |<----------|  |  |EJECT             |  |  |
|        |     |                  |  |           |  |  |                  |  |  |
|        |     |             PAUSE|  |           |  |  |             EJECT|  |  |
|        |     |                  |  |           |  |  |                  |  |  |
|        |     |  +------------+  |  |           |  |  |  +------------+  |  |  |
|        |     |  |  PAUSED    |  |  |           |  |  |  |  OPEN      |  |  |  |
|        |     |  +------------+  |  |           |  |  |  +------------+  |  |  |
|        |     |  | entry/     |  |  |           |  |  |  | entry/     |  |  |  |
|        |     +->| exit/      |--+  |           |  |  +->| exit/      |--+  |  |
|        |        |            |     |           |  |     |            |     |  |
|        |        |            |     |           |  |     |         +--|     |  |
|        |        |            |     |           |  |     |         |  |     |  |
|        |        |            |     |           |  +-----|     LOAD|  |     |  |
|        |        |            |     |           |        |         |  |     |  |
|        |        |            |     |           |        |         +->|     |  |
|        |        |            |     |           |        |            |     |  |
|        |        +------------+     |           |        +------------+     |  |
|        |                           |           |                           |  |
|        +---------------------------+           +---------------------------+  |
|                                                                               |
+-------------------------------------------------------------------------------+
 */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "hsm.h"
#include "player.h"
#include "cd.h"

typedef enum
{
    IDLE_STATE ,
    BUSY_STATE ,
    PLAYING_STATE,
    PAUSED_STATE ,
    CLOSED_STATE ,
    OPEN_STATE ,
}process_state_t;

typedef struct
{
    bool finished;
    bool playing;
    process_track_t* current_track;
}cd_player_t;

static cd_player_t cd_player;

 /*
  *  --------------------- Function prototype ---------------------
  */
static state_machine_result_t busy_handler(state_machine_t* const);
static state_machine_result_t busy_entry_handler(state_machine_t* const);
static state_machine_result_t busy_exit_handler(state_machine_t* const);

static state_machine_result_t idle_handler(state_machine_t* const);
static state_machine_result_t idle_entry_handler(state_machine_t* const);
static state_machine_result_t idle_exit_handler(state_machine_t* const);

static state_machine_result_t playing_handler(state_machine_t* const);
static state_machine_result_t playing_entry_handler(state_machine_t* const);
static state_machine_result_t playing_exit_handler(state_machine_t* const);

static state_machine_result_t paused_handler(state_machine_t* const);
static state_machine_result_t paused_entry_handler(state_machine_t* const);
static state_machine_result_t paused_exit_handler(state_machine_t* const);

static state_machine_result_t closed_handler(state_machine_t* const);
static state_machine_result_t closed_entry_handler(state_machine_t* const);
static state_machine_result_t closed_exit_handler(state_machine_t* const);

static state_machine_result_t open_handler(state_machine_t* const);
static state_machine_result_t open_entry_handler(state_machine_t* const);
static state_machine_result_t open_exit_handler(state_machine_t* const);

static const state_t busy , idle , playing , paused , closed , open;

static const state_t busy =
{
    .handler = busy_handler,
    .enter = busy_entry_handler,
    .exit = busy_exit_handler,
    .parent = NULL,
    .child = &playing,
    .level = 0
};

static const state_t idle =
{
    .handler = idle_handler,
    .enter = idle_entry_handler,
    .exit = idle_exit_handler,
    .parent = NULL,
    .child = &closed,
    .level = 0
};

static const state_t playing =
{
    .handler = playing_handler,
    .enter = playing_entry_handler,
    .exit = playing_exit_handler,
    .parent = &busy,
    .child = NULL,
    .level = 1
};

static const state_t paused =
{
    .handler = paused_handler,
    .enter = paused_entry_handler,
    .exit = paused_exit_handler,
    .parent = &busy,
    .child = NULL,
    .level = 1
};

static const state_t closed =
{
    .handler = closed_handler,
    .enter = closed_entry_handler,
    .exit = closed_exit_handler,
    .parent = &idle,
    .child = NULL,
    .level = 1
};

static const state_t open =
{
    .handler = open_handler,
    .enter = open_entry_handler,
    .exit = open_exit_handler,
    .parent = &idle,
    .child = NULL,
    .level = 1
};

void init_player(player_state_machine_t* const pPlayer)
{
    cd_player.finished = false;
    cd_player.playing = false;
    cd_player.current_track = cd_load(0 , 0);
    pPlayer->Machine.state = &idle;
    pPlayer->Machine.event = 0;
    pPlayer->PlayingTime = 0;
    idle_entry_handler((state_machine_t*)pPlayer);
}

bool is_player_playing()
{
    return cd_player.playing;
}

bool is_player_finished()
{
    return cd_player.finished;
}

void on_player_playing(player_state_machine_t* const pPlayer)
{
    uint32_t t = pPlayer->PlayingTime;
    uint32_t t1 = cd_player.current_track->time;
    printf("\rplaying %s %s %02d:%02d/%02d:%02d" ,
        cd_player.current_track->singer ,
        cd_player.current_track->name ,
        t / 60 , t % 60 ,
        t1 / 60 , t1 % 60);
    if (t == t1)
    {
        cd_player.finished = true;
    }
}

state_machine_result_t busy_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case STOP:
        printf("busy: STOP is processed successfully\n");
        return traverse_state(pState , &closed);

    default:
        printf("busy: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t busy_entry_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("busy: entry handler called\n");
    printf("\nState machine is in busy state\n");
    printf("Supported events\n");
    printf("Press '3' : to pause playing\n");
    printf("Press '2': to stop playing\n");
    return EVENT_HANDLED;
}

state_machine_result_t busy_exit_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("busy: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t idle_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case PLAY:
        printf("idle: PLAY is processed successfully\n");
        return traverse_state(pState , &playing);

    default:
        printf("idle: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t idle_entry_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("idle: entry handler called\n");
    printf("\nState machine is in idle state\n");
    printf("Supported events\n");
    printf("Press '1' : to playing\n");
    printf("Press '4': to open\n");
    return EVENT_HANDLED;
}

state_machine_result_t idle_exit_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("idle: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t playing_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case PAUSE:
        printf("playing: PAUSE is processed successfully\n");
        return traverse_state(pState , &paused);

    case FORWARD:
        printf("playing: FORWARD is processed successfully\n");
        cd_player.current_track = cd_playing_next();
        cd_player.finished = false;
        return traverse_state(pState , &playing);

    case BACK:
        printf("playing: BACK is processed successfully\n");
        cd_player.current_track = cd_playing_pre();
        return traverse_state(pState , &playing);

    default:
        printf("playing: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t playing_entry_handler(state_machine_t* const pState)
{
    (void)(pState);
    cd_player.playing = true;
    printf("playing: entry handler called\n");
    printf("\nState machine is in playing state\n");
    printf("Supported events\n");
    printf("Press '2': to stop playing\n");
    printf("Press '3' : to pause playing\n");
    printf("Press '6': to FORWARD\n");
    printf("Press '7': to BACK\n");
    return EVENT_HANDLED;
}

state_machine_result_t playing_exit_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("playing: exit handler called\n");
    return EVENT_HANDLED;
}


state_machine_result_t paused_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case PAUSE:
        printf("paused: PAUSE is processed successfully\n");
        return traverse_state(pState , &playing);

    default:
        printf("paused: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t paused_entry_handler(state_machine_t* const pState)
{
    (void)(pState);
    cd_player.playing = false;
    printf("paused: entry handler called\n");
    printf("\nState machine is in paused state\n");
    printf("Supported events\n");
    printf("Press '2': to stop playing\n");
    printf("Press '3' : to resume playing\n");
    return EVENT_HANDLED;
}

state_machine_result_t paused_exit_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("playing: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t closed_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case EJECT:
        printf("closed: EJECT is processed successfully\n");
        return traverse_state(pState , &open);

    default:
        printf("closed: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t closed_entry_handler(state_machine_t* const pState)
{
    (void)(pState);
    cd_player.playing = false;
    printf("closed: entry handler called\n");
    printf("\nState machine is in closed state\n");
    printf("Supported events\n");
    printf("Press '1' : to playing\n");
    printf("Press '4': to close\n");
    return EVENT_HANDLED;
}

state_machine_result_t closed_exit_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("closed: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t open_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case EJECT:
        printf("open: EJECT is processed successfully\n");
        return traverse_state(pState , &closed);

    case LOAD:
        printf("open: LOAD is processed successfully\n");
        cd_player.current_track = cd_load(0 , 0);
        return traverse_state(pState , &open);

    default:
        printf("open: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t open_entry_handler(state_machine_t* const pState)
{
    (void)(pState);
    cd_player.playing = false;
    printf("open: entry handler called\n");
    printf("\nState machine is in open state\n");
    printf("Supported events\n");
    printf("Press '1' : to playing\n");
    printf("Press '4': to close\n");
    printf("Press '5': to reload cd\n");
    return EVENT_HANDLED;
}

state_machine_result_t open_exit_handler(state_machine_t* const pState)
{
    (void)(pState);
    printf("open: exit handler called\n");
    return EVENT_HANDLED;
}
