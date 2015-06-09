#include "fsm.h"

int fsm_init(fsm *state_machine, int start_state, int num_states, int num_events, int **transition_matrix)
{
    if (state_machine == 0) {
        return 1;
    }
    
    state_machine->state = start_state;
    state_machine->num_states = num_states;
    state_machine->num_events = num_events;
    state_machine->transition_matrix = transition_matrix;
    
    return 0;
}

 int fsm_next_state(fsm *state_machine, int event)
{
    if (state_machine == 0)
    {
        return 1;
    }
    
    if (event > state_machine->num_events)
    {
        return 2;
    }
    
    state_machine->state = (state_machine->transition_matrix[state_machine->state])[event];
    state_machine->transition_func(state_machine->state);
    
    return 0;
}
