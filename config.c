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

#include "config.h"
#include <avr/eeprom.h>
#include "crc8.h"
#include <string.h>
#include <avr/pgmspace.h>

#include <util/delay.h>
#include "display.h"

/**
 * \file
 * \brief EEPROM configuration
 */

/**
 * \defgroup config EEPROM configuration
 * \brief Functions for reading and writing configuration stored in EEPROM.
 * @{
 */


/**
 * Enable or disable EEPROM data configuration.
 * If this macro is defined program do not read nor write data to EEPROM.
 * Default values are used instead.
 */
#define DISABLE_EEPROM_CONFIG
#undef DISABLE_EEPROM_CONFIG

/** Configuration in EEPROM */
CONFIGURATION EEMEM ee_Config;

/**
 * CRC of the configuration data in EEPROM.
 * Checksum is computed only from \ref ee_Config. Startup message does not count.
 */
uint8_t EEMEM ee_ConfigCRC=0xAB; //0xAB = invalid crc (NOT 00, it's good CRC for empty (all 0's) config)

/**
 * Start-up message on display.
 * Modify to whatever you like.
 */
uint8_t EEMEM ee_Message[64] = " AQUATICUS.INFO";

/** Configuration in RAM */
CONFIGURATION g_Config;

/**
 * \brief Reads configuration data from EEPROM.
 * Checks CRC of config data in EEPROM. Validates CRC and use default values in case of
 * damaged data.
 *
 * \note If macro DISABLE_EEPROM_CONFIG is defined default parameters are used (not read from flash).
 *
 */

inline void ReadConfig()
{
#ifndef DISABLE_EEPROM_CONFIG
	//copy data from EEPROM to RAM
	eeprom_read_block( &g_Config, &ee_Config, sizeof(CONFIGURATION) );

	//read CRC from EEPROM
	uint8_t eeCRC = eeprom_read_byte(&ee_ConfigCRC);

	//compute CRC
	uint8_t crc = crc8((uint8_t*)&g_Config, sizeof(CONFIGURATION) );

	if( crc != eeCRC )
	{
		//invalid data in EEPROM (mostly EEPROM is empty)
		//use default data
		memset( &g_Config, 0, sizeof(CONFIGURATION));

		//real values for Suzuki DL650 '04
		g_Config.MaxGearNumber = MAX_GEAR_NUMBER;
		g_Config.GearLevel[0] = 77;
		g_Config.GearLevel[1] = 97;
		g_Config.GearLevel[2] = 136;
		g_Config.GearLevel[3] = 174;
		g_Config.GearLevel[4] = 210;
		g_Config.GearLevel[5] = 236;

		g_Config.UnknownLevel = 254;

		WriteConfig();
		ledPutc('W');
	}
#else
	//default config (not from EEPROM)
	memset( &g_Config, 0, sizeof(CONFIGURATION));
	g_Config.MaxGearNumber = MAX_GEAR_NUMBER;

	//real values for Suzuki DL650 '04
	g_Config.GearLevel[0] = 77;
	g_Config.GearLevel[1] = 97;
	g_Config.GearLevel[2] = 136;
	g_Config.GearLevel[3] = 174;
	g_Config.GearLevel[4] = 210;
	g_Config.GearLevel[5] = 236;
	g_Config.UnknownLevel = 254;

#endif

}

/**
 * Store configuration data in EEPROM. CRC is computed.
 * \note If macro DISABLE_EEPROM_CONFIG is defined function does nothing.
 */
void WriteConfig()
{
#ifndef DISABLE_EEPROM_CONFIG
	uint8_t crc = crc8( (uint8_t*)&g_Config, sizeof(CONFIGURATION) );

	eeprom_write_block( &g_Config, &ee_Config, sizeof(CONFIGURATION) );

	eeprom_write_byte(&ee_ConfigCRC, crc);
#endif
}

/** @} */
