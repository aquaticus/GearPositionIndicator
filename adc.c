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
 * @brief A/D converter
 */

#include "adc.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "config.h"

/**
 * \defgroup adc A/D reading
 * \brief Reading analog values using built-in A/D converter
 * @{
 */

/**
 * \brief Returns the A/D converter conversion result.
 *
 * \param channel ADC channel, from \b 0 to \b 7.
 *
 * \return 8 bit ADC conversion result.
 *
 * \note ADC unit should be enabled and configured properly.
 */

uint8_t GetADC( uint8_t channel )
{
	ADMUX = channel | _BV( REFS0 ) | _BV(ADLAR); //8bit + AVCC

	//start conversion.
	ADCSRA |= _BV(ADSC);

	 //wait for the 1st result
	loop_until_bit_is_clear( ADCSRA, ADSC );

	return ADCH; //left adjusted = 8 bit
}

/**
 * \brief Returns amount of light in phototransistor.
 *
 * \return 8 bit ADC result.
*/
uint8_t GetLight()
{
	return GetADC( LIGHT_PIN );
}

/**
 * \brief Demo gear box procedure.
 *
 * This is example gear box routine. It does not read real gear box
 * but continuously changes gear every 1s.
 * You must write your own procedure that reads appropriate data
 * from your gear box interface. Typically it reads ADC converter pins.
 *
 * \warning
 * This is an example procedure. It does not return real gear number.
 *
 * \return Gear number. \b 0 for neutral, 1 to 5 for gears.
 * \sa CONFIGURATION GetADC() GetLight()
 */
uint8_t GetGear()
{
	static uint8_t gear; //stores gear number
	static uint8_t counter; //counts number of procedure calls
	static int8_t sign = +1;
	if( counter++ > 200 )
	{
		counter = 0;
		gear += sign;
		if( gear > 4 || gear < 1 )
		{
			sign *= -1; /*change sign*/
		}
	}

	return gear;
}

/*
 * \page gps Gearbox Position Sensor Suzuki DL650
 *
 * \par Voltage levels for gears (pink wire)
 * \b #1 1.4V-1.6V\n
 * \b #2 1.8V-2.0V\n
 * \b #3 2.65V-2.7V\n
 * \b #4 3.3V-3.45V\n
 * \b #5 4.1V-4.25V\n
 * \b #6 4.6V
 *
 * \b 5V level indicates position between gears. When changing gear, it is possible
 * to read 5V, e.g from 2nd to 3rd gear voltage levels can be as follows: 1.8V, 5V, 2.6V.
 *
 * Level can change on higher rpm.\n
 *
 * Neutral position is indicated on blue wire by 9V-10V level.
 *
 * \par Wires
 * - \b pink - gear position
 * - \b blue - neutral position indicator
 * - \b black/white - ground
 */


/*.*****************************************************************************
 * EXAMPLE PROCEDURE FOR SUZUKI DL650 AND 2 ADC LINES.
 *.*****************************************************************************/

/*
 * \brief Return current gear number.
 *
 * Reads ADC and then convert voltage to gear number based on data stored
 * in configuration CONFIGURATION.GearLevel and CONFIGURATION.GearNumber
 *
 * Gear sensor voltage is sampled 10 times every 2ms and then average value
 * is used.
 *
 * It takes ca 20ms to obtain gear number.
 *
 * Voltage levels in g_Config.GearLevel must be sorted, lowest level for neutral
 * highest for 6th.
 *
 * \return Gear number from 0 to 6. \b 0 indicates neutral position, the first gear is 1, 6 sixth.
 *
 *
 *
 * \sa CONFIGURATION GetADC() GetLight()
 * \sa \ref gps IsNeutral()
 *
 */
#if 0
uint8_t GetGear()
{
	const uint8_t NEUTRAL = 50;
	const uint8_t UNKNOWN = 254; //=4.9V
	const uint8_t TOLERANCE = 10;

	static uint8_t gear; //stores gear number when position is unknown (pos between gears)

	/** \par Voltage divider
	 * R1 = 180R\n
	 * R2 = 100R\n
	 * Vout =Vin*R2/(R1+R2)\n
	 * For Vin=10V, ADC reads 3.6V. We treat everything >2V as gearbox neutral position.\n
	 * 2V = 100
	 */

	//Port can be used in digital input mode, but reading analog allows more fine tune.
	if( GetADC(NEUTRAL_PIN) >= NEUTRAL )
	{
		gear=0;
		return gear; //neutral position
	}

	GetADC( GEAR_PIN ); //(the first reading after changing ADC channel may be corrupted
	int adc = GetADC( GEAR_PIN ); //works better (more stable)

	//Gearbox gives 5V if it's in unknown position (when changing gears)
	if( adc >= g_Config.UnknownLevel )
		return gear;

	static uint8_t g;

	for(g=0; g<g_Config.MaxGearNumber-1;g++)
	{
		if( adc-TOLERANCE >= g_Config.GearLevel[5-g] )
		{
			gear = 5-g+1;
			return gear;
		}
	}

	return 1;
}
#endif
/** @} */
