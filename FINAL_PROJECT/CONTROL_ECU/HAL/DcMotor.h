
#ifndef HAL_DCMOTOR_H_
#define HAL_DCMOTOR_H_



#include "../std_types.h"
#include "../Atmega32_Registers.h"
/*MOTOR pins*/
#define MOTOR_IN1 PIN1_ID
#define MOTOR_IN2 PIN2_ID

#define MOTOR_PINS_PORT_ID PORTB_ID
#define MOTOR_PINS_PORT PORTB


/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

/*enum to choose the direction of the motor */
typedef enum{
	CW,ACW
}DcMotor_State;

/*******************************************************************************
 *                                Functions                                  *
 *******************************************************************************/


/*
 * Function name: DcMotor_Init
 * Description: setup the direction for the two motor pins through the GPIO driver.
 				Stop at the DC-Motor at the beginning through the GPIO driver.

   Inputs: none
   return: none
 * */
void DcMotor_Init(void);


/*
 * Function name: DcMotor_Rotate
 * Description:
	➢ The function responsible for rotate the DC Motor CW/ or A-CW or
	stop the motor based on the state input state value.

	➢ Send the required duty cycle to the PWM driver based on the
	required speed value.


	• Inputs:
	➢ state: The required DC Motor state, it should be CW or A-CW or stop.
	DcMotor_State data type should be declared as enum or uint8.

	➢ speed: decimal value for the required motor speed, it should be from
	0 → 100. For example, if the input is 50, The motor should rotate with
	50% of its maximum speed.


	• Return: None
 * */
void DcMotor_Rotate(DcMotor_State state,uint8 speed);


#endif /* HAL_DCMOTOR_H_ */
