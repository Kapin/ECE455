#ifndef _FSM_H
#define _FSM_H

typedef struct
{
    int state;
    int num_states;
    int num_events;
    int **transition_matrix;
    void (*transition_func)(int);
} fsm;

int fsm_init(fsm *state_machine, int start_state, int num_states, int num_events, int **transition_matrix);
int fsm_next_state(fsm *state_machine, int event);

#endif
