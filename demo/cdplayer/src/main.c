#include <stdint.h>
#include <stdio.h>
#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#else
#include <Windows.h>
#endif
#include <stdbool.h>

#include "hsm.h"
#include "player.h"

  /*
   *  --------------------- Global variables ---------------------
   */

   //! Instance of demo_state_machine_t
player_state_machine_t PlayerStateMachine;

//! Semaphore for event synchronization timer, console and main thread.
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

 /** \brief Simulate the timer ISR.
  *
  * This is an one second timer. When Oven is ON it prints the remaining time on console.
  * It also generates the timeout event when on time expires.
  */
void* timer(void* vargp)
{
    (void)(vargp);
    while (1)
    {
        sleep(1);

        if (is_player_playing())
        {
            PlayerStateMachine.PlayingTime++;

            if (is_player_finished())
            {
                printf("\n");
                on_player_timedout(&PlayerStateMachine);  // Generate the timeout event
                PlayerStateMachine.PlayingTime = 0;
#ifndef _WIN32
                sem_post(&Semaphore);   // signal to main thread
#else
                ReleaseSemaphore(Semaphore , 1 , NULL);
#endif // !_WIN32
            }
            else
            {
                on_player_playing(&PlayerStateMachine);  // Generate the timer event
            }
        }
    }
    return NULL;
}

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

        parse_cli(&PlayerStateMachine , input);
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
    init_player(&PlayerStateMachine);

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
    Semaphore = CreateSemaphore(NULL , 2 , 2 , NULL);
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

        if (dispatch_event(&PlayerStateMachine) == EVENT_UN_HANDLED)
        {
            printf("invalid event entered\n");
        }
    }
    return 0;
}
