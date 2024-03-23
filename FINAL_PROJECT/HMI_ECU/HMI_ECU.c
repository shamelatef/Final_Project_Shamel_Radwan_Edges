#include "HAL/keypad.h"
#include "HAL/lcd.h"
#include "MCAL/timer1.h"
#include "MCAL/uart.h"
#include <util/delay.h>
#include <avr/interrupt.h>

/***********************************************************************************
 * *******************************Constants****************************************
 ********************************************************************************* */

#define TICKS_PER_SECOND 31250
#define PASSWORD_SIZE 5
#define MAX_INCORRECT 2

/***********************************************************************************
 * *******************************Global Variables**********************************
 * *********************************************************************************/

volatile uint8 g_tick = 0;
volatile uint8 g_open_door = 0;

volatile uint8 g_error = 0;
volatile uint8 g_n = 0;


/******************************************************************************************
 * *******************************Functions Definition************************************
 * ***************************************************************************************/

void errorMessage(void);
void openDoorMessages(void);


/****************************************************************************************
 * *******************************Main function*****************************************
 * ***************************************************************************************/

int main(void) {

/****************************************************************************************
******************************************Local Variables********************************
******************************************************************************************/

	/* arrays to store the two passwords in*/
	uint8 password_1[PASSWORD_SIZE];
	uint8 password_2[PASSWORD_SIZE];

	/*array to send the password for authentication */
	uint8 password_3[PASSWORD_SIZE + 1];

	/*used in for loops*/
	uint8 i;
	/*Variable to avoid repeating the prompt multiple times
	 * and getting back to it whenever i want*/
	uint8 first_prompt = 0;

	/*variable to get the number of consecutive times password is incorrect*/
	uint8 unmatchedPass_cons = 0;
	/*keep track of which button is pressed*/
	uint8 key;

	/*UART receive*/
	uint8 listen;

/****************************************************************************************
**********************************Drivers Initialization**********************************
*******************************************************************************************/

	UART_ConfigType Uart_Config = { BIT_8, ENABLED_EVEN, BIT_1, 9600 };
	UART_init(&Uart_Config);

	LCD_init();

	/*interrupt enable For Timer1*/
	SREG |= (1 << 7);
	Timer1_ConfigType Timer1_Config = { 0, TICKS_PER_SECOND, PRESCALER_256,
			CTC_OCR1A };


	while (1) {

		first_prompt++;

		/*used to get back to the first prompt as needed*/
		if (first_prompt == 1) {

			/*Step 1*/
			LCD_clearScreen();
			LCD_moveCursor(0, 0);
			LCD_displayString("pls enter pass:");
			LCD_moveCursor(1, 0);
			for (i = 0; key != '='; i++) {
				key = KEYPAD_getPressedKey();

				if ((key <= 9) && (key >= 0)) {
					LCD_displayCharacter('*');
					password_1[i] = key;

				}
				_delay_ms(500);

			}

			_delay_ms(500);

			LCD_clearScreen();
			LCD_moveCursor(0, 0);
			LCD_displayString("pls re-enter the");
			LCD_moveCursor(1, 0);
			LCD_displayString("same pass:");

			/*to avoid getting out of the coming loop instantly
			 * because the last value saved  is '='
			 * */
			key = 0;
			for (i = 0; key != '='; i++) {
				key = KEYPAD_getPressedKey();

				if ((key <= 9) && (key >= 0)) {
					LCD_displayCharacter('*');
					password_2[i] = key;
				}

				_delay_ms(500);

			}
//			_delay_ms(500);

			for (i = 0; i < PASSWORD_SIZE; i++) {
				UART_sendByte(password_1[i]);
				_delay_ms(100);
			}

			for (i = 0; i < PASSWORD_SIZE; i++) {
				UART_sendByte(password_2[i]);
				_delay_ms(100);
			}

		}

		/*end of Step 1*/

		listen = UART_recieveByte();
		/*if the passwords are not matched - go to step 1*/
		if (listen == 'x') {
			first_prompt = 0;
			key = 0;
		}

		/*if the passwords are matched - go to step 2*/

		else if (listen == 'y') {
			for (;;) {

				/*Step 2 - main Options*/

				LCD_clearScreen();
				LCD_moveCursor(0, 0);
				LCD_displayString("+ : Open Door");
				LCD_moveCursor(1, 0);
				LCD_displayString("- : Change Pass");

				key = KEYPAD_getPressedKey();

				/*if the input is '+' */
				if (key == '+') {

					unmatchedPass_cons = 0;
					for (;;) {

						/*enter the password for authentication*/
						LCD_clearScreen();
						LCD_moveCursor(0, 0);
						LCD_displayString("pls enter pas+:"); /*pas+ to differentiate between the prompts*/
						LCD_moveCursor(1, 0);

						_delay_ms(500);

						for (i = 0; key != '='; i++) {
							key = KEYPAD_getPressedKey();
							if ((key <= 9) && (key >= 0)) {
								LCD_displayCharacter('*');
								password_3[i] = key;

							}

							_delay_ms(500);

						}

						LCD_clearScreen();
						password_3[PASSWORD_SIZE] = '+'; /*to let the control know the option*/
						key = 0;

						/*send the password to compare it with the one saved in the EEPROM*/
						for (i = 0; i < PASSWORD_SIZE + 1; i++) {

							UART_sendByte(password_3[i]);
							_delay_ms(100);

						}

						listen = UART_recieveByte();

						/* if the password is incorrect*/
						if (listen == 'u') {

							key = 0;
							unmatchedPass_cons++;
							if (unmatchedPass_cons > MAX_INCORRECT) {

								unmatchedPass_cons = 0;

								Timer1_setCallBack(errorMessage);

								Timer1_init(&Timer1_Config);

								while (g_error != 3){}
								g_error = 0;
								Timer1_deInit();
								break;
							}

						}

						/* if the password is matched*/
						else if (listen == 'm') {

							Timer1_setCallBack(openDoorMessages);
							Timer1_init(&Timer1_Config);
							for (;;) {
								if (g_open_door == 3) {
									Timer1_deInit();
									g_open_door = 0;
									break;
								}
							}
							/*Go to main Options after opening the door*/
							break;

						}

					}/* +*/

				}

				/*if the  option is '-' */
				else if (key == '-') {
					unmatchedPass_cons = 0;
					for (;;) {
						LCD_clearScreen();
						/*pas- to differentiate between the prompts*/
						LCD_displayString("pls enter pas-:");
						LCD_moveCursor(1, 0);
						_delay_ms(500);

						for (i = 0; key != '='; i++) {
							key = KEYPAD_getPressedKey();
							if ((key <= 9) && (key >= 0)) {
								LCD_displayCharacter('*');
								password_3[i] = key;

							}

							_delay_ms(500);

						}
						key = 0;

						LCD_clearScreen();
						password_3[PASSWORD_SIZE] = '-';

						for (i = 0; i < PASSWORD_SIZE + 1; i++) {

							UART_sendByte(password_3[i]);
							_delay_ms(100);

						}

						listen = UART_recieveByte();
						/*if the password is correct, Go to the first prompt :) */
						if (listen == 'c') {
							first_prompt = 0;
							key = 0;
							break;
						}

						/*if not, reenter for 3 times only */
						else if (listen == 'u') {

							key = 0;
							unmatchedPass_cons++;
							if (unmatchedPass_cons > MAX_INCORRECT) {

								unmatchedPass_cons = 0;

								Timer1_setCallBack(errorMessage);
								Timer1_init(&Timer1_Config);

								while (g_error != 3){}
								g_error = 0;
								Timer1_deInit();
								break;
							}

						}


					}
					break;

				}
			}
		}

	}
}

/******************************************************************************************
 * *******************************Functions Declaration***********************************
 * ******************************************************************************************/

/*Function Name: errorMessage
 *Description: Called by timer1 callback function to display error
 *Description: message if the password is entered incorrectly for more than 3 times in a row
 *Parameters: none
 *Return: None */
void errorMessage(void) {

	g_n++;

	if (g_n == 1) {
		LCD_clearScreen();
		LCD_displayString("ERRORRRRRRR");

	}

	else if (g_n == 6) {
		g_error = 3;
		g_n = 0;
	}

}

/*Function Name: errorMessage
 *Description: Called by timer1 callback function to display
 *Description: message while opening door
 *Parameters: none
 *Return: None */

void openDoorMessages(void) {

	g_tick++;
	if (g_tick == 1) {
		LCD_clearScreen();
		LCD_displayString("Door Unlocking");
	}

	else if (g_tick == 2) {
		LCD_clearScreen();
		LCD_displayString("Motor on Hold");

	}

	else if (g_tick == 3) {
		LCD_clearScreen();
		LCD_displayString("Door Locking");

	}

	else if (g_tick == 4) {
		g_open_door = 3;
		g_tick = 0;
	}

}

