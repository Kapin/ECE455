#include <lpc17xx.h>
#include "glcd.h"
#include "fsm.h"


#define DASH_THRESHOLD 3

enum button_states
{
    StartButtonState = 0,
    PushedButtonState = 1,
    DotButtonState = 2,
    DashButtonState = 3,
    NumButtonStates = 4,
};

enum button_events
{
    ButtonPressed = 0,
    ButtonLifted1Period = 1,
    ButtonLifted3Periods = 2,
    ButtonReset = 3,
    NumButtonEvents = 4,
};

enum morse_states
{
    StartState = 0,
    DotState = 1,
    DotDashState = 2,
    DotDashDashState = 3,
    DotDashDashDotState = 4,
    DotDashDashDotDashState = 5,
    DotDashDashDotDashDotState = 6,
    DotDashDashDotDashDotDotState = 7,
    NumStates = 8,
};

enum morse_events
{
    DotEvent = 0,
    DashEvent = 1,
    ResetEvent = 2,
    NumEvents = 3
};

int button_matrix[NumButtonStates][NumButtonEvents] = 
{
    { PushedButtonState, -1, -1, StartButtonState },
    { -1, DotButtonState, DashButtonState, StartButtonState },
    { -1, -1, -1, StartButtonState },
    { -1, -1, -1, StartButtonState }
};

int morse_matrix[NumStates][NumEvents] =
{
    { DotState, StartState, -1 },
    { DotState, DotDashState, -1 },
    { DotState, DotDashDashState, -1 },
    { DotDashDashDotState, StartState, -1 },
    { DotState, DotDashDashDotDashState, -1 },
    { DotDashDashDotDashDotState, DotDashDashState, -1 },
    { DotDashDashDotDashDotDotState, DotDashState, -1 },
    { -1, -1, StartState }
};

fsm button_fsm, morse_fsm;
unsigned long button_push_time = 0L;
unsigned long global_clock = 0L;
int edge_detected = 0;
int prev_val = 0;
int done = 0;

static void setup_fsms(void);
static void setup_button(void);
static void setup_clock(void);
void button_transition(int);
void morse_transition(int);
void handle_tick(void);

int main(void)
{
    SystemInit();
    GLCD_Init();
    setup_fsms();
    setup_clock();
    setup_button();
    
    while(done == 0)
    {
    }
}

static void setup_fsms(void)
{
    if(!fsm_init(&button_fsm, StartButtonState, NumButtonStates, NumButtonEvents, (int**)&button_matrix))
    {
        return;
    }
    
    if(!fsm_init(&button_fsm, StartState, NumStates, NumEvents, (int**)&morse_matrix))
    {
        return;
    }
}

static void setup_button(void)
{
    LPC_PINCON->PINSEL4 &= ~(3 << 20);
    LPC_GPIO2->FIODIR &= ~(1 << 10);
}

static void setup_clock(void)
{
	LPC_TIM_TypeDef *timer;
	GLCD_Clear(White);
	
	timer = (LPC_TIM_TypeDef *) LPC_TIM0;
	timer->PR = 12499;
	timer->MR0 = 2000;
	timer->MCR = (1 << 0) | (1 << 1);
	
	NVIC_EnableIRQ(TIMER0_IRQn);
	timer->TCR = 0x01;
}

void button_transition(int state)
{
    switch(state)
    {
        case StartButtonState:
            break;
        case PushedButtonState:
            button_push_time = global_clock;
            break;
        case DotButtonState:
            GLCD_Clear(White);
            fsm_next_state(&morse_fsm, DotEvent);
            GLCD_DisplayString(0, 0, 1, "dot");
            fsm_next_state(&button_fsm, ButtonReset);
            break;
        case DashButtonState:
            fsm_next_state(&morse_fsm, DashEvent);
            GLCD_DisplayString(0, 0, 1, "dash");
            fsm_next_state(&button_fsm, ButtonReset);
            break;
        default:
            break;
    }
}

void morse_transition(int state)
{
    switch(state)
    {
        case StartState:
            break;
        case DotState:
            GLCD_Clear(White);
            break;
        case DotDashState:
            break;
        case DotDashDashState:
            break;
        case DotDashDashDotState:
            break;
        case DotDashDashDotDashState:
            break;
        case DotDashDashDotDashDotState:
            break;
        case DotDashDashDotDashDotDotState:
            GLCD_DisplayString(0, 0, 1, "Correct");
            fsm_next_state(&morse_fsm, ResetEvent);
            done = 1;
            break;
        default:
            break;
    }
}

void handle_tick(void)
{	
    int button_val;
	LPC_TIM0->IR |= 0x01;
    
    global_clock++;
    
    button_val = ~(LPC_GPIO2->FIOPIN >> 10) & 0x01;
    
    if (edge_detected == 0)
    {
        if (button_val != prev_val)
        {
            edge_detected = 1;
            prev_val = button_val;
            
            if (button_val == 1)
            {
                fsm_next_state(&button_fsm, ButtonPressed);
            }
        }
        
        return;
    }
    
    edge_detected = 0;
    
    if (button_val != prev_val)
    {
        prev_val = button_val;
        fsm_next_state(&button_fsm, ButtonReset);
        return;
    }
    
    if (button_val == 0)
    {
        if (global_clock - button_push_time < DASH_THRESHOLD)
        {
            fsm_next_state(&button_fsm, ButtonLifted1Period);
        }
        else
        {
            fsm_next_state(&button_fsm, ButtonLifted3Periods);
        }
    }
}

__asm void TIMER0_IRQHandler(void)
{
	CPSID i
	PRESERVE8
	IMPORT handle_tick
	PUSH {r4 - r11, lr}
    BL handle_tick
	CPSIE i
    POP {r4 - r11, pc}
}
