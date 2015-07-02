#include <lpc17xx.h>
#include "glcd.h"


#define BUTTON_28 (1 << 28)

static void Setup_INT0(void);
static void Setup_LED(void);
static void Setup_GLCD(void);
static void Trigger_Timer(void);
static void Delay(unsigned int);
void EINT3_IRQHandler_C(void);
void TIMER0_IRQHandler_C(void);

int main()
{
    SystemInit();
    Setup_INT0();
    Setup_LED();
    Setup_GLCD();
    
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
    LPC_TIM0->PR = 25000;
    LPC_TIM0->MR0 = 5000;
    LPC_TIM0->MCR = 0x01;
    
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    LPC_TIM0->TCR = 0x01;
}

static void Delay(unsigned int delay)
{
	unsigned int i = 0, j = 0;
	unsigned volatile int delayCounter = 0;
	
	__disable_irq();
	
	for (; i < delay; i++)
	{
		for (; j < 11050000; j++)
		{
			delayCounter += 10;
			delayCounter *= 10;
			delayCounter = 0;
		}
	}
	
	__enable_irq();
}

void EINT3_IRQHandler_C(void)
{
    LPC_GPIOINT->IO2IntClr |= 1 << 10;
	  NVIC_DisableIRQ(EINT3_IRQn);
	
	  Trigger_Timer();
	
    GLCD_DisplayString(0, 0, 1, "INT0 Fired");
    
    LPC_GPIO1->FIOSET = BUTTON_28;
		
		Delay(500);
	
		LPC_GPIO1->FIOCLR = BUTTON_28;
}

void TIMER0_IRQHandler_C(void)
{
    LPC_TIM0->IR |= 0x01;
		
		GLCD_Clear(White);
		
		NVIC_DisableIRQ(TIMER0_IRQn);
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
