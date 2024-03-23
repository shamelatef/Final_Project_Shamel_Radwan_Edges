

#ifndef TIMER1_H_
#define TIMER1_H_
#include "../std_types.h"

#define NORMAL_MODE
#define COMPARE_MODE

typedef enum
{
	TIMER_STOP,PRESCALER_1,PRESCALER_8,PRESCALER_64,PRESCALER_256,PRESCALER_1024,EXTERNAL_CLOCK_FALLING_EDGE,EXTERNAL_CLOCK_RISING
}Timer1_Prescaler;

typedef enum
{
	NORMAL,PWM_PC_8,PWM_PC_9,PWM_PC_10,CTC_OCR1A,FPWM_8,FPWM_9,F_PWM_10,PWM_PFC_ICR1_BOT,PWM_PFC_OCR1A_BOT,PWM_PC_ICR1_TOP,PWM_PC_OCR1A_TOP,CTC_ICR1,RESERVED,FPWM_ICR1,FPWM_OCR1A

}Timer1_Mode;

typedef struct {
uint16 initial_value;
uint16 compare_value; // it will be used in compare mode only.
Timer1_Prescaler prescaler;
Timer1_Mode mode;
} Timer1_ConfigType;

void Timer1_init(const Timer1_ConfigType * Config_Ptr);
void Timer1_deInit(void);
void Timer1_setCallBack(void(*a_ptr)(void));


#endif /* TIMER1_H_ */
