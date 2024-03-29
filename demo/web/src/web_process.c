#include <stdint.h>
#include <stdio.h>

#include "hsm.h"
#include "web_process.h"

/*
+-----------------------------------------------------------------------------------------------+
|                                                 S0                                            |
+-----------------------------------------------------------------------------------------------+
|  entry/                                                                                       |
|  exit/                                                                                        |
|  H/[foo.equals(0)];                                                                           |
|                                                                                               |
|            +--------------------------+      +--------------------------------------------+   |
|        *-->|            S1            |      |                     S2                     |   |
|            +--------------------------+      +--------------------------------------------+   |
|            | entry/                   |  C   | entry/                                     |   |
|      D     | exit/                    |----->| exit/                                      |   |
|<-----------| H/                       |      | H/[foo.equals(1)];                         |   |
|            |                          |      |                                            |   |
|            |     +----------------+   |  K   |      +------------------------------+      |   |
|            | *-->|      S11       |   |<-----|  *-->|             S21              |      |   |
|            |     +----------------+   |      |      +------------------------------+      |   |
|            |     | entry/         |   |  F   |      | entry/                       |      |   |
|            |     | exit/          |<---------|      | exit/                        |      |   |
|            |     |                |   |      |      |       +--------------+       |      |   |
|            |  B  |                |   |      |      |   *-->|     s211     |       |      |   |
|            |---->|       J        |   |      |   F  |       +--------------+   G   |      |   |
|            |     |   +-------+    |   |-------------------->| entry/       |----------------->|
|            |  +--|   |       |    |   |      |   G  |       | exit/        |       |      |   |
|            |  |  |   |       v    |------------------------>|              |       |  E   |   |
|            |  |  +----------------+   |      |      |   B   |              |<-----------------|
|            |  |                       |      |      |------>|              |       |      |   |
|            |  |  +----------------+   |      |      |       |              |   D   |      |   |
|            | I|  |      S12       |   |      |      |    +--|              |------>|      |   |
|            |  |  +----------------+   |      |      |    |  +--------------+       |      |   |
|            |  |  | entry/         |   |      |      |    |                         |      |   |
|            |  |  |   distAction() |   |      |      |   I|  +--------------+       |      |   |
|            |  |  | exit/          |   |      |      |    |  |     s212     |       |      |   |
|            |  +->|                |   |      |      |    |  +--------------+       |      |   |
|         +--|     |                |   |      |      |    +->| entry/       |       |      |   |
|         |  |     |                |   |  I   |      |       | exit/        |       |      |   |
|         |  |     |                |------------------------>|              |       |      |   |
|        A|  |     |                |   |      |      |       +--------------+       |      |   |
|         |  |     |                |   |      |      |                              |      |   |
|         |  |     +----------------+   |      |      +------------------------------+      |   |
|         +->|                          |      |                                            |   |
|            +--------------------------+      +--------------------------------------------+   |
| A[foo.equals(1)];                                                                             |
+-----------------------------------------------------------------------------------------------+
*/

typedef enum
{
    S0,
    S1,
    S11,
    S12,
    S2,
    S21,
    S211,
    S212,
}process_state_t;

static state_machine_result_t s0_handler(state_machine_t* const);
static state_machine_result_t s0_entry(state_machine_t* const);
static state_machine_result_t s0_exit(state_machine_t* const);

static state_machine_result_t s1_handler(state_machine_t* const);
static state_machine_result_t s1_entry(state_machine_t* const);
static state_machine_result_t s1_exit(state_machine_t* const);

static state_machine_result_t s11_handler(state_machine_t* const);
static state_machine_result_t s11_entry(state_machine_t* const);
static state_machine_result_t s11_exit(state_machine_t* const);

static state_machine_result_t s12_handler(state_machine_t* const);
static state_machine_result_t s12_entry(state_machine_t* const);
static state_machine_result_t s12_exit(state_machine_t* const);

static state_machine_result_t s2_handler(state_machine_t* const);
static state_machine_result_t s2_entry(state_machine_t* const);
static state_machine_result_t s2_exit(state_machine_t* const);

static state_machine_result_t s21_handler(state_machine_t* const);
static state_machine_result_t s21_entry(state_machine_t* const);
static state_machine_result_t s21_exit(state_machine_t* const);

static state_machine_result_t s211_handler(state_machine_t* const);
static state_machine_result_t s211_entry(state_machine_t* const);
static state_machine_result_t s211_exit(state_machine_t* const);

static state_machine_result_t s212_handler(state_machine_t* const);
static state_machine_result_t s212_entry(state_machine_t* const);
static state_machine_result_t s212_exit(state_machine_t* const);

static const state_t s0 , s1 , s11 , s12 , s2 , s21 , s211 , s212;

static const state_t s0 =
{
    .handler = s0_handler,
    .enter = s0_entry,
    .exit = s0_exit,
    .parent = NULL,
    .child = &s1,
    .level = 0
};

static const state_t s1 =
{
    .handler = s1_handler,
    .enter = s1_entry,
    .exit = s1_exit,
    .parent = &s0,
    .child = &s11,
    .level = 1
};

static const state_t s11 =
{
    .handler = s11_handler,
    .enter = s11_entry,
    .exit = s11_exit,
    .parent = &s1,
    .child = NULL,
    .level = 2
};

static const state_t s12 =
{
    .handler = s12_handler,
    .enter = s12_entry,
    .exit = s12_exit,
    .parent = &s1,
    .child = NULL,
    .level = 2
};

static const state_t s2 =
{
    .handler = s2_handler,
    .enter = s2_entry,
    .exit = s2_exit,
    .parent = &s0,
    .child = &s21,
    .level = 1
};

static const state_t s21 =
{
    .handler = s21_handler,
    .enter = s21_entry,
    .exit = s21_exit,
    .parent = &s2,
    .child = &s211,
    .level = 2
};

static const state_t s211 =
{
    .handler = s211_handler,
    .enter = s211_entry,
    .exit = s211_exit,
    .parent = &s21,
    .child = NULL,
    .level = 3
};

static const state_t s212 =
{
    .handler = s212_handler,
    .enter = s212_entry,
    .exit = s212_exit,
    .parent = &s21,
    .child = NULL,
    .level = 3
};

void init_web(web_state_machine_t* const pWeb)
{
    pWeb->machine.state = &s0;
    pWeb->machine.event = 0;
    s0_entry((state_machine_t*)pWeb);
}

state_machine_result_t s0_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case E:
        printf("s0: E is processed successfully\n");
        return traverse_state(pState , &s211);

    default:
        printf("s0: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s0_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s0: entry handler called\n");
    printf("\nState machine is in s0 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s0_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s0: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t s1_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case D:
        printf("s1: D is processed successfully\n");
        return traverse_state(pState , &s0);

    case A:
        printf("s1: A is processed successfully\n");
        return traverse_state(pState , &s1);

    case B:
        printf("s1: B is processed successfully\n");
        return traverse_state(pState , &s11);

    case C:
        printf("s1: C is processed successfully\n");
        return traverse_state(pState , &s2);

    default:
        printf("s1: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s1_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s1: entry handler called\n");
    printf("\nState machine is in s1 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s1_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s1: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t s11_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case J:
        printf("s11: J is processed successfully\n");
        return traverse_state(pState , &s11);

    case I:
        printf("s11: I is processed successfully\n");
        return traverse_state(pState , &s12);

    case G:
        printf("s11: G is processed successfully\n");
        return traverse_state(pState , &s211);

    default:
        printf("s11: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s11_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s11: entry handler called\n");
    printf("\nState machine is in s11 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s11_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s11: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t s12_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case I:
        printf("s12: I is processed successfully\n");
        return traverse_state(pState , &s212);

    default:
        printf("s12: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s12_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s12: entry handler called\n");
    printf("\nState machine is in s12 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s12_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s12: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t s2_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case K:
        printf("s2: K is processed successfully\n");
        return traverse_state(pState , &s1);

    case F:
        printf("s2: F is processed successfully\n");
        return traverse_state(pState , &s11);

    default:
        printf("s2: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s2_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s2: entry handler called\n");
    printf("\nState machine is in s2 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s2_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s2: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t s21_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case B:
        printf("s21: B is processed successfully\n");
        return traverse_state(pState , &s211);

    default:
        printf("s21: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s21_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s21: entry handler called\n");
    printf("\nState machine is in s21 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s21_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s21: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t s211_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    case G:
        printf("s211: G is processed successfully\n");
        return traverse_state(pState , &s0);

    case D:
        printf("s211: D is processed successfully\n");
        return traverse_state(pState , &s21);

    case I:
        printf("s211: I is processed successfully\n");
        return traverse_state(pState , &s212);

    default:
        printf("s211: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s211_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s211: entry handler called\n");
    printf("\nState machine is in s211 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s211_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s211: exit handler called\n");
    return EVENT_HANDLED;
}

state_machine_result_t s212_handler(state_machine_t* const pState)
{
    switch (pState->event)
    {
    default:
        printf("s212: handler does not handle this event\n");
        return EVENT_UN_HANDLED;
    }
}

state_machine_result_t s212_entry(state_machine_t* const pState)
{
    (void)(pState);
    printf("s212: entry handler called\n");
    printf("\nState machine is in s212 state\n");
    return EVENT_HANDLED;
}

state_machine_result_t s212_exit(state_machine_t* const pState)
{
    (void)(pState);
    printf("s212: exit handler called\n");
    return EVENT_HANDLED;
}
