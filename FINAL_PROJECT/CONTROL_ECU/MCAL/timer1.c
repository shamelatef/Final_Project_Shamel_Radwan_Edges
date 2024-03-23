

#include "timer1.h"
#include "../common_macros.h"
#include "../Atmega32_Registers.h"
#include <avr/interrupt.h> /* For ICU ISR */




static volatile void (*g_callBackPtr)(void) = NULL_PTR;

ISR(TIMER1_COMPA_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}



ISR(TIMER1_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}


void Timer1_init(const Timer1_ConfigType * Config_Ptr)
{
	TCNT1 = Config_Ptr->initial_value;
	OCR1A = Config_Ptr->compare_value;
	TIMSK |= (1<<OCIE1A); /* Enable Timer1 Compare A Interrupt */



	TCCR1A = ((TCCR1A&0xFC)|(Config_Ptr->mode&0x03))|(1<<FOC1A);

	TCCR1B = ((TCCR1B&0xE7)|((Config_Ptr->mode<<1) &0x18))|(Config_Ptr->prescaler&(0x07));


	//TCCR1A = (1<<FOC1A);

	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Prescaler = F_CPU/8 CS10=0 CS11=1 CS12=0
	 */
//TCCR1B = (1<<WGM12) | (1<<CS11);
}

void Timer1_deInit(void)
{
	TCNT1 = 0;
	OCR1A = 0;
	CLEAR_BIT(TIMSK,OCIE1A);
	TCCR1A = 0;

	TCCR1B = 0;

	g_callBackPtr = NULL_PTR;
}
void Timer1_setCallBack(void(*a_ptr)(void))
{
	g_callBackPtr= a_ptr;
}
