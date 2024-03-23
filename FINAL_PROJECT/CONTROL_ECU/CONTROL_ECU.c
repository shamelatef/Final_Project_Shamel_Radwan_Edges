


#include "common_macros.h"
#include "MCAL/PWM.h"
#include "MCAL/gpio.h"
#include "HAL/DcMotor.h"
#include "HAL/external_eeprom.h"
#include "MCAL/twi.h"
#include "MCAL/uart.h"
#include "MCAL/timer1.h"
#include "HAL/buzzer.h"
#include <util/delay.h>

/*******************************************************************************************
 * ****************************************Constants***************************************
 **************************************************************************************** */
#define PASSWORD_SIZE 5
#define TICKS_PER_SECOND 31250
#define MAX_INCORRECT 2
#define TWI_MASTER_DEVICE_ADDRESS 10
/*******************************************************************************************
 * **************************************Global Variables**********************************
 * ****************************************************************************************/

volatile uint8 g_tick = 0;
volatile uint8 g_open_door = 0;

volatile uint8 g_error = 0;
volatile uint8 g_n = 0;

/*******************************************************************************************
 * ************************************Functions Definition********************************
 * ****************************************************************************************/
void error(void);
void openDoor(void);

/*******************************************************************************************
 * ***************************************Main function************************************
 * ****************************************************************************************/

int main(void) {
/******************************************************************************************/
/****************************************Local Variables***********************************/
/******************************************************************************************/

	/*Variable to Use in For loops*/
	uint8 i;

	/*Arrays to store the passwords in*/
	uint8 password_1[PASSWORD_SIZE];
	uint8 password_2[PASSWORD_SIZE];

	/*array to get the password for authentication*/
	uint8 password_3[PASSWORD_SIZE + 1];

	/*array to read the password from the eeprom*/
	uint8 read_from_eeprom[PASSWORD_SIZE];

	/*address in the EEBROM*/
	uint16 address = 0x0000;

	/*a counter to see if the passwords are matched*/
	uint8 counter = 0;

	/*Counter to see if the password is entered incorrectly 3 times in a row*/
	uint8 unmatchedPass = 0;

/*******************************************************************************************/
/**********************************Drivers Initialization***********************************/
/*******************************************************************************************/

	/*interrupt enable*/
	SREG |= (1 << 7);

	/*UART initialization*/
	UART_ConfigType Uart_Config = { BIT_8, ENABLED_EVEN, BIT_1, 9600 };
	UART_init(&Uart_Config);

	/*Initializing I2C to use in EEPROM*/
	TWI_BaudRate TWI_BitRate = { 0x02, 0x00 }; /*400khz according to the equation*/
	TWI_ConfigType TWI_Config = { TWI_MASTER_DEVICE_ADDRESS, &TWI_BitRate };
	TWI_init(&TWI_Config);

	/*Initializing I2C to write Device ADDRESS*/
	TWI_start();
	/*device address and write bit =0*/
	TWI_writeByte((TWI_Config.address<<1)|0);

	TWI_stop();


	/*Initializing Timer1 with
	 * pre-scaler of 256
	 * and CTC mode
	 * (top value = 31250 (1 interrupt every 1 seconds)*/
	Timer1_ConfigType Timer1_Config = { 0, TICKS_PER_SECOND, PRESCALER_256,CTC_OCR1A };
	Timer1_init(&Timer1_Config);

	/*Initializing MOTOR using timer0-PWM MODE*/
	DcMotor_Init();
	PWM_Timer0_init();

	/*buzzer*/
	Buzzer_init();


	while (1) {

		/*resetting the address to read from index 0 to index PASSWORD_SIZE*/
		address = 0x0000;

		/*Receiving the Passwords*/
		for (i = 0; i < PASSWORD_SIZE; i++) {
			password_1[i] = UART_recieveByte();
		}

		for (i = 0; i < PASSWORD_SIZE; i++) {
			password_2[i] = UART_recieveByte();

		}

		/*see if the Passwords are matched*/
		counter = 0;
		for (i = 0; i < PASSWORD_SIZE; i++) {
			if (password_1[i] == password_2[i]) {
				counter++;
			}

		}
		if (counter == PASSWORD_SIZE) {
			/* indicating matched passwords sent to the HMI_ECU*/
			UART_sendByte('y');

			/*saving the password in the first 5 bytes in the EEPROM*/
			for (i = 0; i < PASSWORD_SIZE; i++) {
				EEPROM_writeByte(address, password_1[i]);
				_delay_ms(300);

				address++;

			}

			counter = 0;

			/*Getting the password entered for open door/Change Password*/
			for (;;) {
				for (i = 0; i < PASSWORD_SIZE + 1; i++) {
					password_3[i] = UART_recieveByte();
				}

				/*Read the password saved in the EEPROM*/
				address = 0x0000;
				for (i = 0; i < PASSWORD_SIZE; i++) {
					EEPROM_readByte(address, &read_from_eeprom[i]);
					address++;

				}

				/*COmparing the password sent to open the door
				 * with the one saved in the EEPROM
				 */

				for (i = 0; i < PASSWORD_SIZE; i++) {
					if (password_3[i] == read_from_eeprom[i]) {
						counter++;
					}

				}

				/*if the password sent is matched and the order was open door*/
				if (counter == PASSWORD_SIZE
						&& password_3[PASSWORD_SIZE] == '+') {

					Timer1_setCallBack(openDoor);

					UART_sendByte('m'); /*open door option*/
					_delay_ms(100);
					Timer1_setCallBack(openDoor);

					Timer1_init(&Timer1_Config);
					while (g_open_door != 3)
						;
					Timer1_deInit();

					counter = 0;
					unmatchedPass = 0;

				}
				/*if the password sent is matched and the order was change password*/

				else if (counter == PASSWORD_SIZE
						&& password_3[PASSWORD_SIZE] == '-') {
					UART_sendByte('c'); /*change password option*/
					_delay_ms(100);
					unmatchedPass = 0;
					counter = 0;

					break;
				}

				/*Password entered does not match
				 * the one saved in the EEPROM
				 * */
				else if (counter != PASSWORD_SIZE) {
					unmatchedPass++;
					UART_sendByte('u');
					_delay_ms(100);
					counter = 0;
					if (unmatchedPass > MAX_INCORRECT) {
						unmatchedPass = 0;

						Timer1_setCallBack(error);
						Timer1_init(&Timer1_Config);
						while (g_error != 3) {
						}
						g_error = 0;
						Timer1_deInit();

					}

				}

			}

		}

		else {
			UART_sendByte('x'); /* indicating unmatched passwords (password_1 and password_2)*/
		}

	}

}
/******************************************************************************************
 * *******************************Functions Declaration***********************************
 * ****************************************************************************************/

/*Function name: error
 * Description: Error message and buzzer on if 3 times incorrect
 password entered in a row called by timer1 callback
 * return: none
 * parameters: none*/

/*Error message and buzzer on if 3 times incorrect
 * password entered in a row called by timer1 callback
 * */
void error(void) {

	g_n++;
	if (g_n == 1) {

		Buzzer_on();

	} else if (g_n == 6) //60 seconds
			{
		Buzzer_off();
		//UART_sendByte('g');
		//_delay_ms(100);
		g_error = 3;
		g_n = 0;
	}

}

/*Function name: openDoor
 * Description:  to open the door called by timer1 callback
 * return: none
 * parameters: none*/
void openDoor(void) {

	g_tick++;

	if (g_tick == 1) {
		DcMotor_Rotate(CW, 100);
		//UART_sendByte('1'); // "DOor unlocking message"
		_delay_ms(100);

	}

	else if (g_tick == 2)			//15 seconds
			{
		DcMotor_Rotate(ACW, 0);
		//UART_sendByte('2'); // "DOor on hold"

	}

	else if (g_tick == 3) {			// holding for 3 seconds		{

		DcMotor_Rotate(ACW, 100);
		//UART_sendByte('3'); // "DOor locking"

	} else if (g_tick == 4)			//15 seconds
			{

		DcMotor_Rotate(ACW, 0);
		//	UART_sendByte('4');// first prompt =0
		g_open_door = 3;
		g_tick = 0;

	}

}

