#ifndef PLAYER_H
#define PLAYER_H

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

//! Simple hierarchical state machine
typedef struct
{
    state_machine_t Machine;  //!< Abstract state machine
    uint32_t PlayingTime;     //!< Dummy variable
}player_state_machine_t;

extern void init_player(player_state_machine_t* const pPlayer);

static inline void on_player_timedout(player_state_machine_t* const pPlayer)
{
    pPlayer->Machine.event = FORWARD;
}

extern bool is_player_playing();

extern bool is_player_finished();

extern void on_player_playing(player_state_machine_t* const pPlayer);

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
