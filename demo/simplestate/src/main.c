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
#include "demo_process.h"

  /*
   *  --------------------- Global variables ---------------------
   */

   //! Instance of process_t
process_t SampleProcess;

#ifndef _WIN32
//! Semaphore for event synchronization timer, console and main thread.
sem_t Semaphore;
#else
#define sleep(n) Sleep(n * 1000)
HANDLE Semaphore;
#endif


/*
 *  --------------------- Functions ---------------------
 */

 //! Callback function to log the events dispatched by state machine framework.
void event_logger(uint32_t stateMachine , uint32_t state , uint32_t event)
{
    printf("State Machine: %d, State: %d, Event: %d\n" , stateMachine , state , event);
}

//! Callback function to log the result of event processed by state machine
void result_logger(uint32_t state , state_machine_result_t result)
{
    printf("Result: %d, New State: %d\n" , result , state);
}

/** \brief Simulate the timer ISR.
 *
 * This is an one second timer. When process is active it prints the remaining time on console.
 * It also generates the timeout event when process time expires.
 */
void* timer(void* vargp)
{
    (void)(vargp);
    while (1)
    {
        sleep(1);

        if (SampleProcess.Timer > 0)
        {
            SampleProcess.Timer--;

            printf("\rRemaining process time: %d " , SampleProcess.Timer);

            if (SampleProcess.Timer == 0)
            {
                printf("\n");
                on_process_timedout(&SampleProcess);  // Generate the timeout event
#ifndef _WIN32
                sem_post(&Semaphore);   // signal to main thread
#else
                ReleaseSemaphore(Semaphore , 1 , NULL);
#endif // !_WIN32
            }
        }
    }
    return NULL;
}

/** \brief Simulate the user inputs.
 *
 * It waits for the user key (ascii) input from console and pass it to parse_cli
 * to convert it into process_t events. It supports start, stop, pause and resume events.
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

        parse_cli(&SampleProcess , input);
#ifndef _WIN32
        sem_post(&Semaphore);   // signal to main thread
#else
        ReleaseSemaphore(Semaphore , 1 , NULL);
#endif // !_WIN32
    }
}

int main(void)
{
    // Initialize the process state machine.
    init_process(&SampleProcess , 10);

#ifndef _WIN32
    // Create timer and console thread
    pthread_t timer_thread , console_thread;
    pthread_create(&timer_thread , NULL , timer , NULL);
    pthread_create(&console_thread , NULL , console , NULL);
    sem_init(&Semaphore , 0 , 1);
#else
    HANDLE handle[2] = { 0 };
    DWORD threadID = 0;
    handle[0] = CreateThread(NULL , 0 , timer , NULL , 0 , &threadID);
    handle[1] = CreateThread(NULL , 0 , console , NULL , 0 , &threadID);
    Semaphore = CreateSemaphore(NULL , 0 , 1 , NULL);
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

        if (dispatch_event(&SampleProcess.Machine) == EVENT_UN_HANDLED)
        {
            printf("invalid event entered\n");
        }
    }
    return 0;
}
