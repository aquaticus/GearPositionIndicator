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
 * @brief Menu implementation
 */

#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <string.h>
#include "gpi.h"
#include "config.h"
#include "display.h"
#include "button.h"
#include "adc.h"

/**
 * \defgroup menu Menu
 * \brief Menu handling
 * @{
 */


/**
 * Displays text from \a g_TextBuffer and checks button state.
 *
 * @retval BUTTON_SHORT Button pressed short
 * @retval BUTTON_LONG Button pressed long
 * @retval BUTTON_NONE Timeout
 *
 */
uint8_t ShowText()
{
	int offset = 0;
	uint8_t key;

	pCurrentFont = FONTTAB;

	for(uint8_t i=0; i<2; i++)
	{
		do
		{
			ScrollLeft(g_TextBuffer, g_TextBufferLen, &offset );

			key = ButtonCheck();
			if( key == BUTTON_SHORT || key == BUTTON_LONG )
			{
				return key;
			}

			_delay_ms(20);

		} while( offset );
	}

	return BUTTON_NONE;
}

/**
 * Displays text from program memory on display.
 *
 * @param szText Null terminated string in prog memory.
 * @return
 * @sa ShowText()
 */
uint8_t DisplayText_P( PGM_P szText )
{
	g_TextBufferLen = strlen_P( szText );
	memcpy_P( g_TextBuffer, szText, g_TextBufferLen );
	return ShowText();
}

/**
 * Display menu text.
 * @param Name Pointer to text in program memory.
 * @param len1 Length of menu text (\a Name).
 * @param Value String for value in program memory.
 * @param len2 Length of \a Value
 * @return
 * @sa ShowText() DisplayText_P()
 */
uint8_t DisplayMenu( PGM_P Name, size_t len1, PGM_P Value, size_t len2 )
{
	g_TextBuffer[0] = ' ';

	g_TextBufferLen = len1+1;

	memcpy_P( g_TextBuffer+1, Name, len1 );
	g_TextBuffer[g_TextBufferLen++] = SYMBOL_ARROW;

	memcpy_P( g_TextBuffer+g_TextBufferLen, Value, len2 );
	g_TextBufferLen += len2;
	
	return ShowText();
}

/**
 * @brief Display menu and handle user actions.
 *
 * @param index Menu index displayed at the beginning of menu name.
 *
 * @param pMenu Pointer to null terminated string in PROGMEM with menu name and
 * option names divided by '|' character.
 *
 * @param pConfigVar Pointer to variable from \a m_Config.
 *
 * @retval 0 timeout
 * @retval 1 user pressed button for short time (=next menu)
 */
static inline uint8_t ProcessMenu( uint8_t index, PGM_P pMenu, uint8_t* pConfigVar )
{
	struct
	{
		PGM_P pItem;
		size_t len;
	} ItemTab[10]; //1 + 9 options

	int OptCount=1;

	ItemTab[0].pItem = pMenu;

	int i=0;
	char c;

	//parse menu string
	do
	{
		c = pgm_read_byte( pMenu + i);
		if( c == '|' || c == '\0' )
		{
			ItemTab[OptCount].pItem = pMenu+i+1;
			ItemTab[OptCount-1].len = ItemTab[OptCount].pItem - ItemTab[OptCount-1].pItem-1 ;
			OptCount++;
		}

		i++;
	} while( c );

	OptCount--;

	ItemTab[OptCount-1].len = pMenu + i - ItemTab[OptCount-1].pItem - 1;

	//Display menu indicator

	for(uint8_t x=0;x<3;x++)
	{
		//ledNegPutc(index <= 9 ? '0'+index : 'A'+index-10);
		ledPutc(index <= 9 ? '0'+index : 'A'+index-10);
		_delay_ms(60);

		//ledPutc(index <= 9 ? '0'+index : 'A'+index-10);
		ledPutc(' ');
		_delay_ms(60);

	}

	uint8_t CurrentValue=*pConfigVar;
	uint8_t ret;

	do
	{

		ret = DisplayMenu( 	ItemTab[0].pItem, ItemTab[0].len,
				ItemTab[*pConfigVar+1].pItem, ItemTab[*pConfigVar+1].len );

		if( BUTTON_LONG == ret )
		{
			//get the next option to show
			(*pConfigVar)++;

			if( *pConfigVar >= OptCount-1 )
				*pConfigVar = 0;

			AnimateCheck();
		}
		else if( BUTTON_SHORT == ret )
		{
			return 1; //short
		}
	} while( ret == 2 );


	if( CurrentValue != *pConfigVar )
	{
		//store in EEPROM
		WriteConfig();
	}

	return 0;
}

/**
 * Configure gears
 */
#ifdef EXTRA_FUNC
void GearboxConfig()
{
	const int WaitTime = 3000; //ms = 3s
	//TODO Check if level is ok
	const uint8_t LevelOffset = 10; //in 8-bit ADC readings

	pCurrentFont = FONTTAB;

	FlashCharNeg('G',3);

	uint8_t g=0;

	for(g=0;g<MAX_GEAR_NUMBER;g++)
	{
		//flash number
		for (uint8_t x = 0; x < 3; x++)
		{
			ledPutc(g+1);
			_delay_ms(60);

			memset((void*)DisplayBuffer, 0, 8);
			_delay_ms(60);
		}
		ledPutc(g+1);

		_delay_ms( WaitTime );

		//get ADC
		g_Config.GearLevel[g] = GetADC( GEAR_PIN );

		//stop counting gears is two gears got the same voltage levels
		if( g && g_Config.GearLevel[g] >= g_Config.GearLevel[g-1]-LevelOffset &&
			g_Config.GearLevel[g] < g_Config.GearLevel[g-1]+LevelOffset )
		{
			break;
		}
	}

	//write data only if user changed gears
	if( g > 0 )
	{
		//the last gear
		g_Config.MaxGearNumber = g+1;

		WriteConfig();
		DisplayText_P(PSTR(" OK"));
	}
	else
	{
		DisplayText_P(PSTR(" NO CHANGES"));
	}
}
#endif

/**
 * @brief Displays configuration menu.
 */
void ConfigMenu()
{
	typedef struct
	{
		PGM_P MenuString;
		uint8_t* pConfigVar;
	} Menu;

	Menu MenuTree[] =
	{
			//Keep menu names short for easy reading
			{PSTR("SCALE|\034C|\034F"), &g_Config.fTempFahrenheitOn},
			{PSTR("FORMAT|LONG|SHORT"), &g_Config.fTempShortFormatOn},
			{PSTR("TEMP TIMEOUT|NORMAL|SHORT|LONG|OFF"), &g_Config.fTempSmartDisplayTimeout},
			{PSTR("ANIMATON|UP/DOWN|LEFT/RIGHT|NONE"), &g_Config.GearAnimation },
			{PSTR("ROTATE|0\034|90\034|180\034|270\034"), &g_Config.DisplayRotation},
			{PSTR("AUTO BRIGHTNESS|ON|OFF"), &g_Config.fAutoBrightnessOff },
			{PSTR("MIN BRIGHTNESS|0|1|2|3"), &g_Config.MinBrightness},
			{PSTR("STARTUP MSG|OFF|ON"), &g_Config.fStartupMessageOn},
	};


	pCurrentFont = FONTTAB;

	FlashCharNeg('C',3);

	size_t i=0;
	while(1)
	{
		if( BUTTON_NONE == ProcessMenu( 1+i, MenuTree[i].MenuString, MenuTree[i].pConfigVar ) )
			break;

		i++;
		if( i >= sizeof(MenuTree)/sizeof(Menu) )
			i=0;
	}

	return;
}

/** @} */
