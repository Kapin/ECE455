#ifndef _FSM_H
#define _FSM_H

struct fsm
{
    int state;
    int num_states;
    int num_events;
    int **transition_matrix;
    void (*transition_func)(int);
};

int fsm_init(struct fsm *state_machine, int start_state, int num_states, int num_events, int **transition_matrix);
int fsm_next_state(struct fsm *state_machine, int event);

#endif
