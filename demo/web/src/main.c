/*
  *  --------------------- INCLUDE FILES ---------------------
  */
#include <stdint.h>
#include <stdio.h>

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#else
#include <Windows.h>
#endif

#include "hsm.h"
#include "web_process.h"

  /*
   *  --------------------- Global variables ---------------------
   */

   //! Instance of demo_state_machine_t
web_state_machine_t webStateMachine;

//! Semaphore for event synchronization timer, console and main thread.
#ifndef _WIN32
//! Semaphore for event synchronization timer, console and main thread.
sem_t Semaphore;
#else
#define sleep(n) Sleep(n * 1000)
HANDLE Semaphore;
#endif

/** \brief Simulate the user inputs.
 *
 * It waits for the user key (ascii) input from console and pass it to parse_cli
 * to convert it into demo_event_t events. It supports start, stop, door open and door close events.
 */
void* console(void* vargp)
{
    (void)(vargp);
    while (1)
    {
        // Get input from console
        char input = getchar();

        // ignore new line input
        if ((input == '\n') || (input == '\r'))
        {
            continue;
        }

        parse_cli(&webStateMachine , input);
#ifndef _WIN32
        sem_post(&Semaphore);   // signal to main thread
#else
        ReleaseSemaphore(Semaphore , 1 , NULL);
#endif // !_WIN32
    }
}

int main(void)
{
    // Initialize the demo state machine.
    init_web(&webStateMachine);

#ifndef _WIN32
    // Create timer and console thread
    pthread_t console_thread;
    pthread_create(&console_thread , NULL , console , NULL);
    sem_init(&Semaphore , 0 , 1);
#else
    HANDLE handle = { 0 };
    DWORD threadID = 0;
    handle = CreateThread(NULL , 0 , console , NULL , 0 , &threadID);
    Semaphore = CreateSemaphore(NULL , 1 , 1 , NULL);
#endif // !_WIN32

    if (NULL == Semaphore)
    {
        return -1;
    }

    while (1)
    {
#ifndef _WIN32
        sem_wait(&Semaphore);   // Wait for event
#else
        if (WAIT_OBJECT_0 != WaitForSingleObject(Semaphore , INFINITE))
        {
            continue;
        }
#endif // !_WIN32

        if (dispatch_event(&webStateMachine.machine) == EVENT_UN_HANDLED)
        {
            printf("invalid event entered\n");
        }
    }
    return 0;
}
