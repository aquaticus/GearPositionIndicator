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
 * \brief EEPROM config related defines
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

/**
 * \name Scrolling speed
 * @{
 */
#define CONF_SCROLL_NORMAL 0
#define CONF_SCROLL_SLOW   1
#define CONF_SCROLL_FAST   2
///@}

/**
 * \name Gear animation type
 * @{
 */
#define CONF_ANIM_UPDOWN 0
#define CONF_ANIM_LEFTRIGHT 1
#define CONF_ANIM_NONE 2
///@}

/**
 * \name Rotation angle of the display.
 * @{
 */
#define CONF_ROTATE_0 0
#define CONF_ROTATE_90 1
#define CONF_ROTATE_180 2
#define CONF_ROTATE_270 3
///@}

/**
 * \name Menu scroll speed
 * Delay in milliseconds affecting menu scrolling speed.
 * @{
 */
#define SCROLL_SLOW_MS 40
#define SCROLL_NORMAL_MS 25
#define SCROLL_FAST_MS 15
/**@}*/

/**
 * Maximum number of gears including neutral.
 */
#define MAX_GEAR_NUMBER 6

/**
 * Structure stores configuration settings.
 *
 * \sa ReadConfig() WriteConfig()
 *
 * \warning Default values are set to 0 in case of EEPROM corruption.
 * Every new entry in config should assume its default value as 0.
 */

typedef struct tagConfiguration
{
	uint8_t fTempFahrenheitOn; ///< Use Fahrenheit scale instead of Celsius
	uint8_t fTempShortFormatOn; ///< Use short temperature format
	uint8_t fTempSmartDisplayTimeout; ///< Don't use smart temperature display
	uint8_t ScrollingSpeed; ///< Speed of scrolling
	uint8_t GearAnimation; ///< Animation mode

	uint8_t fAutoBrightnessOff; ///< Auto brightness off
	uint8_t DisplayRotation; ///< Display orientation

	uint8_t MinBrightness; ///< Minimum brightness of the display. The range is from 0 to 7 \b not 0-15!

	uint8_t fUseComma; ///< Use comma (,) instead of dot (.) as decimal separator

	uint8_t fStartupMessageOn; ///< Display message at startup.

	uint8_t MaxGearNumber; ///< Number of gears

	uint8_t UnknownLevel; ///< ADC reading for unknown gear position

	uint8_t GearLevel[MAX_GEAR_NUMBER]; ///< Voltage levels for gears
} CONFIGURATION;

extern CONFIGURATION g_Config;
extern uint8_t ee_Message[];

void ReadConfig();
void WriteConfig();

#endif
