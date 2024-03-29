#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "hsm.h"

#define MAX_HIERARCHICAL_LEVEL 5
static state_t* state_hierachical[MAX_HIERARCHICAL_LEVEL];

state_machine_result_t dispatch_event(state_machine_t* const hsm)
{
    state_machine_result_t result = EVENT_HANDLED;

    const state_t* state = hsm->state;

    do
    {
        if (0 == hsm->event)
        {
            break;
        }

        if (NULL == state->handler)
        {
            return EVENT_UN_HANDLED;
        }

        result = state->handler(hsm);

        switch (result)
        {
        case EVENT_HANDLED:
            hsm->event = 0;
            break;
        case TRIGGERED_TO_SELF:
            break;
        case EVENT_UN_HANDLED:
            {
                do
                {
                    if (NULL == state->parent)
                    {
                        return EVENT_UN_HANDLED;
                    }
                    state = state->parent;
                } while (NULL == state->handler);
                continue;
            }
        
        default:
            return result;
        }

    } while (1);

    return EVENT_HANDLED;
}

state_machine_result_t traverse_state(state_machine_t* const hsm , const state_t* target)
{
    const state_t* source = hsm->state;
    hsm->state = target;    // save the target node
    uint32_t index = 0;

    if (source->level > target->level)
    {
        while (source->level > target->level)
        {
            source->exit(hsm);
            source = source->parent;
        }
    }
    else if (source->level < target->level)
    {
        while (source->level < target->level)
        {
            state_hierachical[index++] = (state_t*)target;
            target = target->parent;
        }
    }

    while (source->parent != target->parent)
    {
        if (source->exit)
        {
            source->exit(hsm);
        }
        
        source = source->parent;

        state_hierachical[index++] = (state_t*)target;
        target = target->parent;
    }

    if (source->exit)
    {
        source->exit(hsm);
    }

    if (target->enter)
    {
        target->enter(hsm);
    }

    while (index)
    {
        index--;
        if (state_hierachical[index] && state_hierachical[index]->enter)
        {
            state_hierachical[index]->enter(hsm);
        }
    }
    return EVENT_HANDLED;
}
