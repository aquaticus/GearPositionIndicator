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
 * \defgroup temp Temperature reading
 * \brief DS18B20 temperature sensor related functions.
 * @{
 */

/**
 * @file
 * @brief DS18B20 temperature sensor functions
 */

#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "temp.h"
#include "gpi.h"
#include <string.h>
#include "crc8.h"
#include "display.h"
#include "config.h"
#include "adc.h"

/**
 * Current temperature in Celsius * 10.
 * Updated every time by GetTempConversionResult()
 * It can be \ref INVALID_TEMP in case of reading problems.
 *
 * @sa GetTempConversionResult()
 */

int16_t g_nTemperature = INVALID_TEMP;

/**
 * @brief Format temperature into string.
 *
 * Result stored in g_TextBuffer. Length of text is set g_TextBufferLen.
 * String is formatted based on configuration settings.
 *
 * \return Length of the formatted string without terminating 0.
 * @sa g_Config
*/

int FormatTemperature()
{
	int temp;
	PGM_P szInvalidTemp = " ??~C";

	if( g_Config.fTempFahrenheitOn )
	{
		//convert from Fahrenheit
		temp = g_nTemperature * 9;
		temp /= 5;
		temp += 32;
	}
	else
	{
		temp = g_nTemperature;
	}


	if( g_nTemperature == INVALID_TEMP )
	{
		//memcpy_P( g_TextBuffer, szInvalidTemp, sizeof(szInvalidTemp )-1 );
		strcpy_P( g_TextBuffer, szInvalidTemp);
		return sizeof(szInvalidTemp)-1;
	}

	g_TextBuffer[0] = ' ';

	size_t index;

	if( g_Config.fTempShortFormatOn )
	{
		//short format

		int t = temp/10;
		int f = abs(temp % 10);
		if( t > 0 )
		{
			t+= (f >= 5 ? 1:0);
		}
		else
		{
			t-= (f >= 5 ? 1:0);
		}
		itoa( t, g_TextBuffer+1, 10 );
		index = strlen( g_TextBuffer + 1 ) + 1;
	}
	else
	{
		//long format

		itoa( temp/10, g_TextBuffer+1, 10 );
		index = strlen( g_TextBuffer + 1 ) + 1;

		g_TextBuffer[index++] = g_Config.fUseComma ? ',' : '.';

		g_TextBuffer[index++] = '0' + temp % 10;
	}

	g_TextBuffer[index++] = SYMBOL_DEG; //deg symbol
	g_TextBuffer[index++] = g_Config.fTempFahrenheitOn ? 'F' : 'C';

	g_TextBufferLen = index;

	return index;
}

/**
 * @brief Displays current temperature on LED.
 *
 * Scrolling speed have to be the same as in TemperatureLoop() function.
 * In general, it should be impossible to see the difference when temperature is shown
 * by both functions.
 */
void DisplayTemperature()
{
	int offset = 0;

	pCurrentFont = FONTTAB;

	if( GetTempConversionResult() )
		g_nTemperature = INVALID_TEMP;

	FormatTemperature();

	do
	{
		ScrollLeft(g_TextBuffer, g_TextBufferLen, &offset);
		SCROLL_DELAY;
	} while( offset );
}

/**
 * @brief Start DS18B20 temperature conversion.
 *
 * It takes 750ms to convert temperature. Conversion result must be obtained
 * using GetTempConversionResult().
 *
 * @warning Function disable and enable interrupts.
 * @return 0 for success.
 */
uint8_t StartTempConversion()
{
	uint8_t err = ow_reset();

	if( err )
	{
		return err; //error
	}

	ow_byte_wr( OW_SKIP_ROM );
	ow_byte_wr( OW_CONVERT );

	return 0; //success
}

/**
 * @brief Convert raw DS18B20 data to temperature integer.
 * Function works properly only for 12 bit result.
 *
 * @param raw Pointer to raw data (2 first bytes) of 12 bit reading.
 * @return Temperature * 10 in Celsius degrees, e.g. -10.6C = -106
 */
inline int16_t Raw2Temp( uint8_t* raw )
{
	int16_t temp;

	temp = raw[0] | (raw[1] << 8);
	temp *= 10;
	temp /= 16;

	return temp;
}

/**
 * @brief Get temperature from DS18B20 sensor.
 *
 * As result \ref g_nTemperature is set.
 * Temperature conversion must be initiated by StartTempConversion(). Function can read
 * temperature 750ms after calling StartTempConversion();
 *
 * @warning Function disable and enable interrupts.
 *
 * @retval 0 success.
 * @retval 1 reset error
 * @retval 2 CRC error
 *
 * @sa StartTempConversion()
 */
uint8_t GetTempConversionResult()
{
	const uint8_t BytesToRead = 9;
	uint8_t rawtemp[ BytesToRead ];
	uint8_t err;

	err = ow_reset();
	if( err )
		return err;

	ow_byte_wr( OW_SKIP_ROM );
	ow_byte_wr( OW_READ );

	for( uint8_t i = 0; i < BytesToRead; i++ )
	{
		rawtemp[i] = ow_byte_rd();
	}

	if ( crc8( rawtemp, BytesToRead-1 ) != rawtemp[BytesToRead-1] )
	{
		return 2; //crc error
	}

	//convert temp from raw data to int
	g_nTemperature = Raw2Temp(rawtemp);

	return 0; //success
}

/**
 * \name 1wire low level
 * Based on Peter Dannegger code
 * @{
 */

/**
 * 1-wire bus reset
 * @return
 */
uint8_t ow_reset(void)
{
	uint8_t err;

	OW_OUT_LOW();
	OW_DIR_OUT(); // pull OW-Pin low for 480us
	_delay_us(480);

	cli(); //disable interrupts

	// set Pin as input - wait for clients to pull low
	OW_DIR_IN(); // input
	OW_OUT_HIGH(); //internal pull-up

	_delay_us(64); // was 66
	err = OW_GET_IN(); // no presence detect
	// if err!=0: nobody pulled to low, still high

	sei(); //re-enable interrupts

	// after a delay the clients should release the line
	// and input-pin gets back to high by pull-up-resistor
	_delay_us(480 - 64); // was 480-66
	if (OW_GET_IN() == 0)
	{
		err = 1; // short circuit, expected low but got high
	}

	return err;
}

uint8_t ow_bit_io(uint8_t b)
{
	cli();

	OW_OUT_LOW(); //pull-up

	OW_DIR_OUT(); // drive bus low
	_delay_us(2); // T_INT > 1usec accoding to timing-diagramm
	if (b & 1)
	{
		OW_DIR_IN(); // to write "1" release bus, resistor pulls high

		OW_OUT_HIGH(); //internal pull-up
	}

	// "Output data from the DS18B20 is valid for 15usec after the falling
	// edge that initiated the read time slot. Therefore, the master must
	// release the bus and then sample the bus state within 15ussec from
	// the start of the slot."
	_delay_us(15 - 2 - OW_CONF_DELAYOFFSET);

	if (OW_GET_IN() == 0)
	{
		b = 0; // sample at end of read-timeslot
	}

	_delay_us(60 - 15 - 2 + OW_CONF_DELAYOFFSET);

	OW_OUT_HIGH(); //pull-up

	OW_DIR_IN();

	sei();

	_delay_us(OW_RECOVERY_TIME); // may be increased for longer wires

	return b;
}

uint8_t ow_byte_wr(uint8_t b)
{
	uint8_t i = 8, j;

	do
	{
		j = ow_bit_io(b & 1);
		b >>= 1;
		if (j)
		{
			b |= 0x80;
		}
	} while (--i);

	return b;
}

uint8_t ow_byte_rd( void )
{
	// read by sending only "1"s, so bus gets released
	// after the init low-pulse in every slot
	return ow_byte_wr( 0xFF );
}

///@}
///@}
