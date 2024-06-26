#ifndef DEMO_PROCESS_H
#define DEMO_PROCESS_H

//! List of process events
typedef enum
{
  START = 1,
  STOP,
  PAUSE,
  RESUME,
  TIMEOUT,
}process_event_t;

/*
 *  --------------------- STRUCTURE ---------------------
 */

//! process state machine
typedef struct
{
  state_machine_t Machine;      //!< Abstract state machine
  uint32_t Set_Time;    //! Set time of a process
  uint32_t Resume_Time; //!< Remaining time when the process is paused
  uint32_t Timer;       //!< Process timer
}process_t;

/*
 *  --------------------- External function prototype ---------------------
 */

extern void init_process(process_t* const pProcess, uint32_t processTime);

/*
 *  --------------------- Inline functions ---------------------
 */

 // process APIs

static inline void start_process(process_t* const pProcess)
{
  pProcess->Machine.event = START;
}

static inline void stop_process(process_t* const pProcess)
{
  pProcess->Machine.event = STOP;
}

static inline void pause_process(process_t* const pProcess)
{
  pProcess->Machine.event = PAUSE;
}

static inline void resume_process(process_t* const pProcess)
{
  pProcess->Machine.event = RESUME;
}

static inline void on_process_timedout(process_t* const pProcess)
{
  pProcess->Machine.event = TIMEOUT;
}

/** \brief Parses the user keyboard input and calls the respective API,
 *  to trigger the events to state machine.
 *
 * \param pProcess process_t* const instance of process_t state machine.
 * \param input char  user input
 *
 */
static inline void parse_cli(process_t* const pProcess, char input)
{
  switch(input)
  {
  case 's':
  case 'S':
    start_process(pProcess);
    break;

  case 'q':
  case 'Q':
    stop_process(pProcess);
    break;

  case 'p':
  case 'P':
    pause_process(pProcess);
    break;

  case 'r':
  case 'R':
    resume_process(pProcess);
    break;

  default:
    printf("Not a valid event\n");
    break;
  }
}

#endif // DEMO_PROCESS_H
