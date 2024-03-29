#ifndef PLAYER_H
#define PLAYER_H

/**
 * \file
 * \brief hierarchical state machine demo

 * \author  Nandkishor Biradar
 * \date    03 February 2020

 *  Copyright (c) 2018-2020 Nandkishor Biradar
 *  https://github.com/kiishor

 *  Distributed under the MIT License, (See accompanying
 *  file LICENSE or copy at https://mit-license.org/)
 */

/*
 *  --------------------- ENUMERATION ---------------------
 */

//! List of supported events
//! 

typedef enum
{
    PLAY = 1 ,
    STOP ,
    PAUSE ,
    EJECT ,
    LOAD ,
    FORWARD ,
    BACK
}player_event_t;


/*
 *  --------------------- STRUCTURE ---------------------
 */

//! Simple hierarchical state machine
typedef struct
{
    state_machine_t Machine;  //!< Abstract state machine
    uint32_t PlayingTime;     //!< Dummy variable
}player_state_machine_t;

/*
 *  --------------------- External function prototype ---------------------
 */

extern void init_player(player_state_machine_t* const pPlayer);

static inline void on_player_timedout(player_state_machine_t* const pPlayer)
{
    pPlayer->Machine.event = FORWARD;
}

extern bool is_player_playing();

extern bool is_player_finished();

extern void on_player_playing(player_state_machine_t* const pPlayer);

/*
 *  --------------------- Inline functions ---------------------
 */

/** \brief Parses the user keyboard input and calls the respective API,
 *  to trigger the events to state machine.
 *
 * \param pDemo demo_state_machine_t* const: instance of \ref demo_state_machine_t state machine.
 * \param input char:  user input
 *
 */
static inline void parse_cli(player_state_machine_t* pPlayer , char input)
{
  switch(input)
  {
  case '1':
      pPlayer->Machine.event = PLAY;
    break;

  case '2':
      pPlayer->Machine.event = STOP;
    break;

  case '3':
      pPlayer->Machine.event = PAUSE;
    break;

  case '4':
      pPlayer->Machine.event = EJECT;
      pPlayer->PlayingTime = 0;
      break;

  case '5':
      pPlayer->Machine.event = LOAD;
      pPlayer->PlayingTime = 0;
      break;

  case '6':
      pPlayer->Machine.event = FORWARD;
      pPlayer->PlayingTime = 0;
      break;

  case '7':
      pPlayer->Machine.event = BACK;
      pPlayer->PlayingTime = 0;
      break;

  default:
    printf("Not a valid event\n");
    break;
  }
}

#endif // PLAYER_H
