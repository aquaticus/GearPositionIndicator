/*
                          _   _                  _        __
   __ _  __ _ _   _  __ _| |_(_) ___ _   _ ___  (_)_ __  / _| ___
  / _` |/ _` | | | |/ _` | __| |/ __| | | / __| | | '_ \| |_ / _ \
 | (_| | (_| | |_| | (_| | |_| | (__| |_| \__ \_| | | | |  _| (_) |
  \__,_|\__, |\__,_|\__,_|\__|_|\___|\__,_|___(_)_|_| |_|_|  \___/
           |_|

 Copyright (c) 2012, All Right Reserved, http://aquaticus.info

 THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

*/

/**
 * @file
 * @brief Button functions
 */

#include "button.h"
#include <avr/io.h>
#include "gpi.h"
#include <util/delay.h>

/**
 * Status of the CheckButton() function.
 *
 * @sa CheckButton()
 */
static uint8_t ButtonState = 0;

/**
 * @brief Reset internal state of CheckButton() function.
 *
 * It is needed to reset state before check state loop.
 */
void ResetButtonState()
{
	ButtonState=0;
}

/**
 * @brief Check if button is down.
 *
 * The button is pressed when it's state
 * has changed from 0 to 1. Function blocks execution of
 * the program for \ref BUTTON_DEBOUNCE_TIME.
 *
 * @retval BUTTON_NONE nothing
 * @retval BUTTON_DOWN button down
 *
 * @sa ButtonCheck()
 */
uint8_t ButtonDown()
{
	ResetButtonState();

	ButtonCheck();

	_delay_ms( BUTTON_DEBOUNCE_TIME );

	return ButtonCheck();
}


/**
 * @brief Check button state.
 *
 * The function must be called min. every 20ms to detect button state changes. For longer
 * times it can not detect all the states. For shorter periods it can detect false states.
 *
 * It keeps its internal state and thanks to that it can detect
 * short or long button push. It also can detect when button was pushed
 * (falling edge). The function does not block execution of the main program (no any delays
 * inside).
 *
 * It is recommended to use ResetButtonState() function to initialize internal state of
 * the function.
 *
 * When used press the button for long time the sequence of returning values is:
 * -# BUTTON_NONE
 * -# BUTTON_DOWN
 * -# BUTTON_LONG
 *
 * When button is pushed for short time, the sequence is:
 * -# BUTTON_NONE
 * -# BUTTON_DOWN
 * -# BUTTON_SHORT
 *
 * To detect \a BUTTON_DOWN state function must be called at least twice.
 *
 * @retval BUTTON_NONE button untouched
 * @retval BUTTON_SHORT button pressed shortly
 * @retval BUTTON_LONG button pressed for long time
 * @retval BUTTON_DOWN bottom was pressed, the state was changed from untouched to pressed.
 *
 * @par Example
 * @code
 * uint8_t b;
 * while(1)
 * {
 *   b = ButtonCheck();
 *   if( b == BUTTON_SHORT )
 *   	puts("Short");
 *   else if( b == BUTTON_LONG )
 *   	puts("Long");
 *
 *   _delay_ms(20);
 * }
 * @endcode
 *
 * @sa ResetButtonState()
 */

uint8_t ButtonCheck()
{
	const uint8_t LongLimit = 40;
	static uint8_t counter = 0;

	if( ButtonState == 0 && bit_is_clear(BUTTON_PORT, BUTTON_PIN) )
	{
		ButtonState++;
		return BUTTON_NONE;
	}

	if( ButtonState == 1 && bit_is_clear(BUTTON_PORT, BUTTON_PIN) )
	{
		counter=0;
		ButtonState++; //button is in low state
		return BUTTON_DOWN;
	}

	if( ButtonState == 2 && bit_is_clear(BUTTON_PORT, BUTTON_PIN) )
	{
		if( counter > LongLimit )
		{
			ButtonState = 5;
			return  BUTTON_LONG;
		}

		counter++;

		return BUTTON_NONE;
	}

	if( ButtonState == 2 && bit_is_set(BUTTON_PORT, BUTTON_PIN) )
	{
		counter++;
		ButtonState++;

		return 0;
	}

	if( ButtonState == 3 && bit_is_set(BUTTON_PORT, BUTTON_PIN) )
	{
		counter++;
		ButtonState = 0;

		if( counter > LongLimit )
			return BUTTON_LONG;
		else
			return BUTTON_SHORT;
	}

	// stage 5 is when user keeps button down log long time
	if( ButtonState == 5 && bit_is_clear(BUTTON_PORT, BUTTON_PIN) )
	{
		return BUTTON_NONE; //do nothing
	}

	ButtonState = 0;

	return BUTTON_NONE;
}
