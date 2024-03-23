
#ifndef HAL_BUZZER_H_
#define HAL_BUZZER_H_

#include "../MCAL/gpio.h"
#define BUZZER_PORT PORTC_ID
#define BUZZER_PIN PIN2_ID

void Buzzer_init(void);
void Buzzer_on(void);
void Buzzer_off(void);


#endif /* HAL_BUZZER_H_ */
