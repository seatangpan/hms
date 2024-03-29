#ifndef WEB_PROCESS_H
#define WEB_PROCESS_H

typedef enum
{
    A = 1 ,
    B ,
    C ,
    D ,
    E ,
    F ,
    G ,
    H ,
    I ,
    J ,
    K ,
}process_event_t;

//! Simple hierarchical state machine
typedef struct
{
    state_machine_t machine;  //!< Abstract state machine
}web_state_machine_t;

void init_web(web_state_machine_t* const pWeb);

static inline void parse_cli(web_state_machine_t* pWeb , char input)
{
    switch (input)
    {
    case 'a':
    case 'A':
        pWeb->machine.event = A;
        break;
    
    case 'b':
    case 'B':
        pWeb->machine.event = B;
        break;

    case 'c':
    case 'C':
        pWeb->machine.event = C;
        break;

    case 'd':
    case 'D':
        pWeb->machine.event = D;
        break;

    case 'e':
    case 'E':
        pWeb->machine.event = E;
        break;

    case 'f':
    case 'F':
        pWeb->machine.event = F;
        break;

    case 'g':
    case 'G':
        pWeb->machine.event = G;
        break;

    case 'h':
    case 'H':
        pWeb->machine.event = H;
        break;

    case 'i':
    case 'I':
        pWeb->machine.event = I;
        break;

    case 'j':
    case 'J':
        pWeb->machine.event = J;
        break;

    case 'k':
    case 'K':
        pWeb->machine.event = K;
        break;

    default:
        printf("Not a valid event\n");
        break;
    }
}

#endif // DEMO_PROCESS_H
