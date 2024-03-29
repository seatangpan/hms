#ifndef HSM_H
#define HSM_H

typedef enum
{
    EVENT_HANDLED ,
    EVENT_UN_HANDLED ,
    TRIGGERED_TO_SELF ,
}state_machine_result_t;

typedef struct hierarchical_state state_t;
typedef struct state_machine_t state_machine_t;
typedef state_machine_result_t(*state_handler) (state_machine_t* const hsm);

//! Hierarchical state structure
struct hierarchical_state
{
    state_handler handler;
    state_handler enter;
    state_handler exit;
    const state_t* const parent;
    const state_t* const child;
    uint32_t level;
};

//! Abstract state machine structure
struct state_machine_t
{
    uint32_t event;          //!< Pending Event for state machine
    const state_t* state;    //!< State of state machine.
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

state_machine_result_t dispatch_event(state_machine_t* const hsm);

state_machine_result_t traverse_state(state_machine_t* const hsm , const state_t* target);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif