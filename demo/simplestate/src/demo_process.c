#include <stdint.h>
#include <stdio.h>

#include "hsm.h"
#include "demo_process.h"

/*
 *  --------------------- ENUMERATION ---------------------
 */

//! List of states the process state machine
typedef enum
{
  IDLE_STATE,
  ACTIVE_STATE,
  PAUSE_STATE
}process_state_t;

/*
 *  --------------------- Function prototype ---------------------
 */

static state_machine_result_t idle_handler(state_machine_t* const State);
static state_machine_result_t idle_entry_handler(state_machine_t* const State);
static state_machine_result_t idle_exit_handler(state_machine_t* const State);

static state_machine_result_t active_handler(state_machine_t* const State);
static state_machine_result_t active_entry_handler(state_machine_t* const State);
static state_machine_result_t active_exit_handler(state_machine_t* const State);

static state_machine_result_t paused_handler(state_machine_t* const State);
static state_machine_result_t paused_entry_handler(state_machine_t* const State);
static state_machine_result_t paused_exit_handler(state_machine_t* const State);

/*
 *  --------------------- Global variables ---------------------
 */

static const state_t Process_States[] =
{
  [IDLE_STATE] = {
    .handler = idle_handler,
    .enter   = idle_entry_handler,
    .exit    = idle_exit_handler,
  },

  [ACTIVE_STATE] = {
    .handler = active_handler,
    .enter = active_entry_handler,
    .exit = active_exit_handler,
  },

  [PAUSE_STATE] = {
    .handler = paused_handler,
    .enter = paused_entry_handler,
    .exit = paused_exit_handler,
  }
};

/*
 *  --------------------- Functions ---------------------
 */

void init_process(process_t* const pProcess, uint32_t processTime)
{
  pProcess->Machine.state = &Process_States[IDLE_STATE];
  pProcess->Machine.event = 0;
  pProcess->Set_Time = processTime;
  pProcess->Resume_Time = 0;

  idle_entry_handler((state_machine_t *)pProcess);
}

static state_machine_result_t idle_entry_handler(state_machine_t* const pState)
{
  process_t* const pProcess = (process_t*)pState;
  pProcess->Timer = 0;  // Stop process timer

  printf("Entering to idle state\n");
  printf("Supported events\n");
  printf("'s' : Start process\n");
  return EVENT_HANDLED;
}

static state_machine_result_t idle_handler(state_machine_t* const pState)
{
  switch(pState->event)
  {
  case START:
    return traverse_state(pState, &Process_States[ACTIVE_STATE]);

  default:
    return EVENT_UN_HANDLED;
  }
  return EVENT_HANDLED;
}

static state_machine_result_t idle_exit_handler(state_machine_t* const pState)
{
  process_t* const pProcess = (process_t*)pState;
  pProcess->Timer = pProcess->Set_Time;
  printf("Exiting from idle state\n");
  return EVENT_HANDLED;
}

static state_machine_result_t active_entry_handler(state_machine_t* const pState)
{
  (void)(pState);
  printf("Entering to active state\n");
  printf("Supported events\n");
  printf("'q' : stop process\n");
  printf("'p' : Pause process\n");
  return EVENT_HANDLED;
}

static state_machine_result_t active_handler(state_machine_t* const pState)
{
  switch(pState->event)
  {
  case STOP:
    return traverse_state(pState, &Process_States[IDLE_STATE]);

  case PAUSE:
    return traverse_state(pState, &Process_States[PAUSE_STATE]);

  case TIMEOUT:
    return traverse_state(pState, &Process_States[IDLE_STATE]);

  default:
    return EVENT_UN_HANDLED;
  }
  return EVENT_HANDLED;
}

static state_machine_result_t active_exit_handler(state_machine_t* const pState)
{
  (void)(pState);
  printf("Exiting from Active state\n");
  return EVENT_HANDLED;
}

static state_machine_result_t paused_entry_handler(state_machine_t* const pState)
{
  process_t* const pProcess = (process_t*)pState;
  pProcess->Resume_Time = pProcess->Timer;  // Save remaining time
  pProcess->Timer = 0;    // Stop the process timer

  printf("Entering to pause state\n");
  printf("Supported events\n");
  printf("'q' : stop process\n");
  printf("'r' : resume process\n");
  return EVENT_HANDLED;
}

static state_machine_result_t paused_handler(state_machine_t* const pState)
{
  process_t* const pProcess = (process_t*)pState;
  switch(pState->event)
  {
  case STOP:
    return traverse_state(pState, &Process_States[IDLE_STATE]);

  case RESUME:
    pProcess->Timer = pProcess->Resume_Time;
    return traverse_state(pState, &Process_States[ACTIVE_STATE]);

  default:
    return EVENT_UN_HANDLED;
  }
  return EVENT_HANDLED;
}

static state_machine_result_t paused_exit_handler(state_machine_t* const pState)
{
  (void)(pState);
  printf("Exiting from paused state\n");
  return EVENT_HANDLED;
}
