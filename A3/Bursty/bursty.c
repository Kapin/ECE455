#include <lpc17xx.h>
#include "glcd.h"


#define BUTTON_28 (1 << 28)

static void Setup_INT0(void);
static void Setup_LED(void);
static void Setup_GLCD(void);
static void Trigger_Timer(void);
void EINT3_IRQHandler_C(void);
void TIMER0_IRQHandler_C(void);

unsigned int burst_counter = 3;

int main()
{
    SystemInit();
    Setup_INT0();
    Setup_LED();
    Setup_GLCD();
    Trigger_Timer();
    
    while (1)
    {
    }
}

static void Setup_INT0(void)
{
    LPC_PINCON->PINSEL4 &= ~(3 << 20);
    LPC_GPIO2->FIODIR &= ~(1 << 10);
    LPC_GPIOINT->IO2IntEnF |= 1 << 10;
    NVIC_EnableIRQ(EINT3_IRQn);
}

static void Setup_LED(void)
{
    LPC_GPIO1->FIODIR |= 0xB0000000;
    LPC_GPIO1->FIOCLR = BUTTON_28;
}

static void Setup_GLCD(void)
{
    GLCD_Init();
    GLCD_Clear(White);
}

static void Trigger_Timer(void)
{
    LPC_TIM0->TCR = 0x02;
    LPC_TIM0->PR = 12499;
    LPC_TIM0->MR0 = 20000;
    LPC_TIM0->MCR = 0x03;
    
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    LPC_TIM0->TCR = 0x01;
}

void EINT3_IRQHandler_C(void)
{
    LPC_GPIOINT->IO2IntClr |= 1 << 10;
    
    burst_counter -= 1;
    
    if (burst_counter == 0)
    {
        NVIC_DisableIRQ(EINT3_IRQn);
    }
    
    LPC_GPIO1->FIOSET = BUTTON_28;
    LPC_GPIO1->FIOCLR = BUTTON_28;
}

void TIMER0_IRQHandler_C(void)
{
    LPC_TIM0->IR |= 0x01;
    
    burst_counter = 3;
    
    NVIC_EnableIRQ(EINT3_IRQn);
}

__asm void EINT3_IRQHandler(void)
{
    PRESERVE8
    IMPORT EINT3_IRQHandler_C
    PUSH {r4 - r11, lr}
    BL EINT3_IRQHandler_C
    POP {r4 - r11, pc}
}

__asm void TIMER0_IRQHandler(void)
{
    PRESERVE8
    IMPORT TIMER0_IRQHandler_C
    PUSH {r4 - r11, lr}
    BL TIMER0_IRQHandler_C
    POP {r4 - r11, pc}
}
