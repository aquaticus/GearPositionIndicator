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
 * \file
 * \brief main() function and other high level functions.
 *
 * \mainpage
 * The control code for the Gear Position Indicator (GPI).\n
 * For hardware configuration see \ref hardware page.
 *
 * For schematics see http://aquaticus.info/gpi
 *
 * \warning
 * You must modify the code of GetGear() function and schematic to match your gearbox.
 */

/**
 * \page hardware Hardware
 * \brief Hardware configuration
 *
 * \par Microcontroller
 * * Chip: ATmega 88/168/328 (88p/168p/328p)
 * * System clock: 8MHz (Set fuse bits to use internal RC oscillator)
 *
 * \par External components (in reference schematic)
 * * PC0 - auxiliary pin #1
 * * PC1 - 5V ADC input
 * * PC2 - button
 * * PC3 - 12V ADC input
 * * PC4 - light sensor
 * * PC5 - 1wire interface for DS18B20 temperature sensor
 * * PC6 - auxiliary pin #2
 * * PC7 - auxiliary pin #3
 *
 * \par LED columns
 * * PD0 (OUT) - LED column 6\n
 * * PD1 (OUT) - LED column 3\n
 * * PD2 (OUT) - LED column 1\n
 * * PD3 (OUT) - LED column 2\n
 * * PD4 (OUT) - LED column 4\n
 * * PD5 (OUT) - LED column 8\n
 * * PD6 (OUT) - LED column 5\n
 * * PD7 (OUT) - LED column 7\n
 *
 * \par LED rows
 * * PB0 (IN) - LED row 1\n
 * * PB1 (IN) - LED row 6\n
 * * PB2 (IN) - LED row 4\n
 * * PB3 (IN) - LED row 2\n
 * * PB4 (IN) - LED row 3\n
 * * PB5 (IN) - LED row 5\n
 * * PB6 (IN) - LED row 8\n
 * * PB7 (IN) - LED row 7\n


 \par Notes
 Enable pull-up resistors for PC4, PC5, PC2.\n
 PB6 and PB7 configured as I/O lines.\n
*/

/**
 * \page how How to use
 * \brief Short manual.
 *
 * If you press the button quickly current temperature is shown.\n
 * If you press the button longer, you go to configuration menu.
 *
 * \par Configuration menu
 * Pressing button quickly to skip current option and show the next one. To change
 * the option press the button until you see a check mark. To quick configuration menu
 * wait.
 */

/**
 * \page self Self test
 * \brief Self test mode description.
 *
 * Press the button down and turn the power on. You will see LED display test and than
 * light sensor test.
 */

/**
 * \page fonts How to modify fonts
 * \brief Short instruction how to modify/create fonts
 * * Edit \c symbols8x8.txt file. The run \c fontconvert.pl utility.
 * * New file \c symbols8x8.c containing \c C table will be generated.
 * * Recompile project.
 */

/**
 * \file symbols8x8.c
 * \brief Fonts as \c C table.
 *
 * File generated from \c symbols8x8.txt
 * \warning Do not edit. Instead edit \c symbols8x8.txt
 */


#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <math.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "gpi.h"
#include "crc8.h"
#include "config.h"
#include "display.h"
#include "menu.h"
#include "temp.h"
#include "button.h"
#include "adc.h"

/// Program name, date and time of compilation.
const PROGMEM char REVISION[] = "@(#)GPI " __DATE__ " " __TIME__;

/**
 * @brief Initialize internal hardware
 *
 * The following things are setup:
 * * I/O lines
 * * Pull-up resistors
 * * ADC
 * * Timers
 * * Interrupts
 *
 */

inline void InitHardware()
{
	// I/O lines
	DDRB = 0xFF; //inputs
	DDRD = 0xFF; //outputs
	DDRC = 0x00; //inputs

	PORTC = 1 << BUTTON_PIN; //pull-up for button
	PORTC |= 1 << LIGHT_PIN; //pull-up for phototransistor

	// ADC
	// Prescaler: 128 division factor that gives 8000000/128 ADC 62kHz clock
	ADCSRA |= _BV( ADEN ) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);

	//timer0 prescaler
	TCCR0A = 0; //normal mode
	TCCR0B = _BV(CS00); //no prescaling (=1)

	//enable overflow interrupt for timer0
	TIMSK0 = _BV( TOIE0 );
}


/**
 * @brief Main loop for displaying gear number.
 *
 * Temperature conversion is initiated every 5s
 *
 * @retval 0 The button was pressed shortly
 * @retval 1 The button was pressed for long time.
 * @retval BUTTON_TEMP_MODE Temperature must be displayed
 */

inline uint8_t GearLoop()
{
	const uint8_t LoopTime = 54; //[ms]

	uint8_t gear;
	uint8_t b;
	uint8_t prev_gear;
	uint16_t display_counter=0;
	uint16_t timeout; //number of cycles per one minute

	//display initially gear number without any animation
	gear = prev_gear = GetGear();
	ledPutc(SYMBOL_GEAR_NUMBER+prev_gear);

	//{PSTR("Auto Temp|30sec|10sec|1min|OFF"), &g_Config.fTempSmartDisplayTimeout},
	//timeout
	//1 loop cycle is about 180ms
	switch( g_Config.fTempSmartDisplayTimeout )
	{
		default:
		case 0:
			timeout = 30000/LoopTime;
			break;

		case 1:
			timeout = 10000/LoopTime;
			break;

		case 2:
			timeout = 60000/LoopTime;
			break;

		case 3:
			timeout = 0; //disable
			break;
	}


	do
	{
		b = ButtonCheck();
		if( b == BUTTON_SHORT || b == BUTTON_LONG )
			return b;

		//display temperature if auto-temp enabled
		if( timeout && (gear == 0 || gear == g_Config.MaxGearNumber) &&
				display_counter > timeout )
		{
			return BUTTON_TEMP_MODE;
		}

		//initialize temperature conversion
		StartTempConversion();


		gear = GetGear(); //takes 20ms

		if( prev_gear != gear )
		{
			//anim takes 8*20ms
			Animate( prev_gear, gear );
			prev_gear = gear;
		}

		if( gear == 0 || gear == g_Config.MaxGearNumber )
			display_counter++;
		else
			display_counter = 0;

	} while(1);

	return 0;
}

/**
 * @brief Displays temperature on LED.
 *
 * @retval BUTTON_SHORT Short button click
 * @retval BUTTON_LONG Long button click
 * @retval BUTTON_GEAR_MODE Timeout, gear must be shown.
 *
 */
static inline uint8_t TemperatureLoop()
{
	uint8_t gear;
	uint8_t b;
	int offset=0;

	pCurrentFont = FONTTAB;

	while(1)
	{
		//get the last conversion result
		if( GetTempConversionResult() )
			g_nTemperature = INVALID_TEMP;

		FormatTemperature();

		do
		{
			gear = GetGear(); //=20ms

			//if driver changed gear exit
			if( gear != 0 && gear != g_Config.MaxGearNumber)
				return BUTTON_GEAR_MODE;

			b = ButtonCheck();
			//button pushed - exit
			if( b == BUTTON_SHORT || b == BUTTON_LONG )
				return b;

			//scroll temp
			ScrollLeft(g_TextBuffer, g_TextBufferLen, &offset);

			SCROLL_DELAY;

		} while( offset );

		//Initiate temperature conversion
		//conversion takes 750ms
		//(it can be called more often then 750ms)
		StartTempConversion();
	}

	return BUTTON_GEAR_MODE;
}

/**
 * Self test.
 */
static inline void SelfTest()
{
	uint8_t i;

	ledNegPutc(' ');
	_delay_ms(500);

	ledPutc(' ');

	//horizontal line
	for(i=0;i<8;i++)
	{
		DisplayBuffer[i]=0xFF;
		_delay_ms(50);
		DisplayBuffer[i] = 0;
	}


	//vertical line
	for(i=0;i<8;i++)
	{
		for(uint8_t y=0;y<8;y++)
		{
			DisplayBuffer[y]=1<<i;
		}
		_delay_ms(50);
	}


	BrightnessLevel();

	DisplayTemperature();
}



/**
 * Main program loop 
 */
int main()
{
	uint8_t key;

	ReadConfig();

	//TODO Initialize watchdog
	InitHardware();

	sei(); //enable global interrupts

	//TODO: Jump directly into gear loop in case of watchdog reset

	//Display startup message
	if( g_Config.fStartupMessageOn )
	{
		ledPuts_EE( (uint8_t*)&ee_Message );
	}

	//initialize temperature conversion
	StartTempConversion();

	//check state of button line at power up
	if( bit_is_clear(BUTTON_PORT, BUTTON_PIN) )
	{
		SelfTest();
	}

	while(1)
	{
		key = GearLoop();

		if( BUTTON_TEMP_MODE == key )
		{
			key = TemperatureLoop();

			if( BUTTON_LONG == key )
				ConfigMenu();
		}
		else if( BUTTON_SHORT == key )
		{
			DisplayTemperature();
		}
		else if (BUTTON_LONG == key)
		{
			ConfigMenu();
		}
	}
}

