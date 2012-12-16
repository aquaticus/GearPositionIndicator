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
 * \brief LED display
 */

#include "display.h"
#include "config.h"
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "button.h"
#include <avr/eeprom.h>

/**
 * \defgroup display Display
 * \brief Manipulate data on LED display.
 * @{
 */


/**
 * \brief Points to current font table.
 */
PGM_VOID_P pCurrentFont = FONTTAB;

/**
 * \brief Data to be displayed in hardware compatible format.
 *
 * Do not copy data to this table. Data is copied from DisplayBuffer
 * after displaying one symbol on LED (prevents blinking).
 *
 * \note This buffer is used by interrupt, never copy any data directly to this buffer.
 */

volatile uint8_t HardwareBuffer[8];

/**
 * \brief Frame buffer.
 *
 * Data is stored in simple pixel format, rows and columns are in proper order.
 * Copy data to be displayed here.
 */

volatile uint8_t DisplayBuffer[8];


/**
 * \brief Buffer used to format text.
 *
 * Data length is stored in \a g_TextBufferLen.
 */

char g_TextBuffer[ TEXTBUFFER_SIZE ];

/**
 * Length of the data in \a g_TextBuffer.
 */

uint8_t g_TextBufferLen = 0;

/// LED display brightness.
/// From 0 (dark) to 15 (bright).
volatile uint8_t g_LedBrightness = BRIGHTNESS_MAX;

/**
 * \brief Swap bits in byte to match hardware configuration.
 *
 * \param col Data to swap (pixels in column).
 *
 * \sa SwapColBitsMirror()
 */
inline uint8_t SwapColBits( uint8_t col )
{
	uint8_t o=0;

	if( col & 0b10000000 ) o |= 0b00000100;
	if( col & 0b01000000 ) o |= 0b00000010;
	if( col & 0b00100000 ) o |= 0b00001000;
	if( col & 0b00010000 ) o |= 0b00000001;
	if( col & 0b00001000 ) o |= 0b01000000;
	if( col & 0b00000100 ) o |= 0b00010000;
	if( col & 0b00000010 ) o |= 0b00100000;
	if( col & 0b00000001 ) o |= 0b10000000;

	return o;
}

/**
 * \brief Swap bits in byte to match hardware configuration as mirror
 * \param col Data to swap (pixels in column).
 *
 * \sa SwapColBits()
 *
 */
inline uint8_t SwapColBitsMirror( uint8_t col )
{
	uint8_t o=0;

	if( col & 0b00000001 ) o |= 0b00000100;
	if( col & 0b00000010 ) o |= 0b00000010;
	if( col & 0b00000100 ) o |= 0b00001000;
	if( col & 0b00001000 ) o |= 0b00000001;
	if( col & 0b00010000 ) o |= 0b01000000;
	if( col & 0b00100000 ) o |= 0b00010000;
	if( col & 0b01000000 ) o |= 0b00100000;
	if( col & 0b10000000 ) o |= 0b10000000;

	return o;
}

/**
 * Copy data from frame buffer to hardware buffer for
 * 0deg display.
 *
 * \sa CopyDisplayToHardware180()
 */
void CopyDisplayToHardware0()
{
	HardwareBuffer[0] = SwapColBits( DisplayBuffer[0] );
	HardwareBuffer[1] = SwapColBits( DisplayBuffer[5] );
	HardwareBuffer[2] = SwapColBits( DisplayBuffer[3] );
	HardwareBuffer[3] = SwapColBits( DisplayBuffer[1] );
	HardwareBuffer[4] = SwapColBits( DisplayBuffer[2] );
	HardwareBuffer[5] = SwapColBits( DisplayBuffer[4] );
	HardwareBuffer[6] = SwapColBits( DisplayBuffer[7] );
	HardwareBuffer[7] = SwapColBits( DisplayBuffer[6] );
}

/**
 * Copy data from frame buffer to hardware buffer for
 * 180deg display.
 *
 * \sa CopyDisplayToHardware0()
 */
void CopyDisplayToHardware180()
{
	HardwareBuffer[6] = SwapColBitsMirror( DisplayBuffer[0] );
	HardwareBuffer[4] = SwapColBitsMirror( DisplayBuffer[5] );
	HardwareBuffer[5] = SwapColBitsMirror( DisplayBuffer[3] );
	HardwareBuffer[7] = SwapColBitsMirror( DisplayBuffer[1] );
	HardwareBuffer[1] = SwapColBitsMirror( DisplayBuffer[2] );
	HardwareBuffer[2] = SwapColBitsMirror( DisplayBuffer[4] );
	HardwareBuffer[0] = SwapColBitsMirror( DisplayBuffer[7] );
	HardwareBuffer[3] = SwapColBitsMirror( DisplayBuffer[6] );
}

/**
 * Copy data from frame buffer to hardware buffer for
 * 90deg display.
 *
 * \sa CopyDisplayToHardware0()
 */

void CopyDisplayToHardware90()
{
	uint8_t tmp[8];
	uint8_t col,row;

	memset( tmp, 0, 8 );

	for(col=0;col<8;col++)
	{
		for(row=0; row<8; row++)
		{
			if( DisplayBuffer[col] & (1<<row) )
				tmp[row] |= 1<<col;
		}
	}

	HardwareBuffer[0] = SwapColBitsMirror( tmp[0] );
	HardwareBuffer[1] = SwapColBitsMirror( tmp[5] );
	HardwareBuffer[2] = SwapColBitsMirror( tmp[3] );
	HardwareBuffer[3] = SwapColBitsMirror( tmp[1] );
	HardwareBuffer[4] = SwapColBitsMirror( tmp[2] );
	HardwareBuffer[5] = SwapColBitsMirror( tmp[4] );
	HardwareBuffer[6] = SwapColBitsMirror( tmp[7] );
	HardwareBuffer[7] = SwapColBitsMirror( tmp[6] );
}

/**
 * Copy data from frame buffer to hardware buffer for
 * 270deg display.
 *
 * \sa CopyDisplayToHardware0() CopyDisplayToHardware90() CopyDisplayToHardware180()
 */

void CopyDisplayToHardware270()
{
	uint8_t tmp[8];
	uint8_t col,row;

	memset( tmp, 0, 8 );

	for(col=0;col<8;col++)
	{
		for(row=0; row<8; row++)
		{
			if( DisplayBuffer[col] & (1<<(7-row)) )
				tmp[row] |= 1<<col;
		}
	}
	HardwareBuffer[0] = SwapColBits( tmp[0] );
	HardwareBuffer[1] = SwapColBits( tmp[5] );
	HardwareBuffer[2] = SwapColBits( tmp[3] );
	HardwareBuffer[3] = SwapColBits( tmp[1] );
	HardwareBuffer[4] = SwapColBits( tmp[2] );
	HardwareBuffer[5] = SwapColBits( tmp[4] );
	HardwareBuffer[6] = SwapColBits( tmp[7] );
	HardwareBuffer[7] = SwapColBits( tmp[6] );
}


/**
 * \name Frame buffer manipulation
 * @{
 */

void ShiftLeft( uint8_t offset, PGM_P OldData, PGM_P NewData )
{
	uint8_t o,n;

	for(uint8_t i=0;i<8;i++)
	{
		o = pgm_read_byte( OldData++ );
		n = pgm_read_byte( NewData++ );

		DisplayBuffer[i] = o << offset;
		DisplayBuffer[i] |= n >> (8-offset);
	}
}


void ShiftRight( uint8_t offset, PGM_P OldData, PGM_P NewData )
{
	uint8_t o,n;

	for(uint8_t i=0;i<8;i++)
	{
		o = pgm_read_byte( OldData++ );
		n = pgm_read_byte( NewData++ );

		DisplayBuffer[i] = o >> offset;
		DisplayBuffer[i] |= n << (8-offset);
	}
}


void ShiftUp( uint8_t offset, PGM_P OldData, PGM_P NewData )
{
	uint8_t y=0;
	for(uint8_t i=offset;i<8;i++)
	{
		DisplayBuffer[y++] = pgm_read_byte( NewData+i );
	}

//	if( offset <= 8 )
//		DisplayBuffer[y++] = 0;

	for(uint8_t i=0;i<offset-1;i++)
	{
		DisplayBuffer[y++] = pgm_read_byte( OldData+i );
	}
}

void ShiftDown( uint8_t offset, PGM_P OldData, PGM_P NewData )
{
	uint8_t y=0;

	for(uint8_t i=offset;i<8;i++)
	{
		DisplayBuffer[y++] = pgm_read_byte( NewData+i );
	}

	for(uint8_t i=0;i<offset;i++)
	{
		DisplayBuffer[y++] = pgm_read_byte( OldData+i );
	}

}

///@}

/**
 * \name Text scrolling
 * @{
 */

/**
 * Scrolls text from right to left.
 *
 * \param szText Pointer to text in RAM. It may be handy to add one space
 * at the beginning got cool looking scrolling.
 *
 * \param Len Length of the text without terminating null.
 *
 * \param pOffset Offset, from 0 to 8 * \a Len
 *
 * \par Example
 * char test[] = " HELLO";
 * int offset=0;
 * do
 * {
 * 	ScrollLeft( test, sizeof(test)-1, &offset);
 * } while( offset );
*/
int ScrollLeft( const char* szText, int Len, int* pOffset )
{
	int c = *pOffset / 8;
	int bit = *pOffset % 8;

	if( c >= Len )
	{
		*pOffset = 0;
		return *pOffset;
	}

	char c1 = szText[c++];
	char c2 = c <= Len-1 ? szText[c] : ' '/*extra space at the end*/;

	ShiftLeft( bit, pCurrentFont+c1*8, pCurrentFont+c2*8 );

	(*pOffset)++;

	return *pOffset;
}

/**
 * \brief Scrolls test upwards.
 * \param szText Text to scroll. NULL at the end is not required.
 * \param Len Length of \a szText
 * \param[in,out] pOffset Offset in pixels.
 * \return New offset value.
 *
 * \sa ScrollDown() ScrollLLeft() ScrollRight()
 */
int ScrollUp( const char* szText, int Len, int* pOffset )
{
	int c = *pOffset / 8;
	int bit = *pOffset % 8;

	if( c >= Len )
	{
		*pOffset = 0;
		return *pOffset;
	}

	char c1 = szText[c++];
	char c2 = c <= Len-1 ? szText[c] : ' '/*extra space at the end*/;

	ShiftUp( bit, pCurrentFont+c2*8, pCurrentFont+c1*8 );

	(*pOffset)++;

	return *pOffset;
}

int ScrollDown( const char* szText, int Len, int* pOffset )
{
	int c = *pOffset / 8;
	int bit = 8-*pOffset % 8;

	if( c >= Len )
	{
		*pOffset = 0;
		return *pOffset;
	}

	char c1 = szText[c++];
	char c2 = c <= Len-1 ? szText[c] : ' ' /*extra space at the end*/;

	ShiftDown( bit, pCurrentFont+c1*8, pCurrentFont+c2*8 );

	(*pOffset)++;

	return *pOffset;
}

///@}

/**
 * \brief Flash character on LED display.
 *
 *
 * \param c ASCII code
 * \param n Number of blinks
 *
 * \sa FlashCharNeg()
 */
#if EXTRA_FUNCS
void FlashChar(char c, uint8_t n)
{
	for (uint8_t x = 0; x < n; x++)
	{
		ledPutc(c);
		_delay_ms(60);

		ledPutc(' ');
		_delay_ms(60);

	}
	ledPutc(c);
}
#endif

/**
 * \brief Flash character on LED display.
 * The character is displayed then it's negative.
 *
 * \param c ASCII code
 * \param n Number of blinks
 *
 * \sa FlashChar()
 */

void FlashCharNeg(char c, uint8_t n)
{
	for (uint8_t x = 0; x < n; x++)
	{
		ledPutc(c);
		_delay_ms(60);

		ledNegPutc(c);
		_delay_ms(60);
	}
	ledPutc(c);
}

/**
 * \brief Display character on LED display
 * \param c ASCII code. Only characters that are defined in font table can be displayed.
 *
 *
 * Character data is taken from address pointed by \a pCurrentFont.
 *
 * \sa FONTTAB pCurrentFont ledNegPutc()
 */
void ledPutc( char c /** ASCII code */)
{
	memcpy_P( (void*)DisplayBuffer, pCurrentFont + c * 8, 8);
}

/**
 * \brief Display character on LED display in \b negative
 * \param c ASCII code.
 *
 * For more info see ledPutc() function.
 *
 * \sa ledPutc()
 */
void ledNegPutc( char c /** ASCII code */)
{
	for(uint8_t i=0;i<8;i++)
	{
		DisplayBuffer[i] = ~pgm_read_byte(pCurrentFont + c * 8 + i);
	}
}

/**
 * Display text stored in EEPROM memory using current font.
 *
 * \param szText Text to display in EEPROM. Must be terminated by 0 or 0xFF.
 *
 * \retval BUTTON_SHORT Button pressed short
 * \retval BUTTON_LONG Button pressed long
 * \retval BUTTON_NONE Timeout
 */
uint8_t ledPuts_EE( const uint8_t* szText )
{
	int offset = 0;
	uint8_t key;

	//copy data from EEPROM
	for(g_TextBufferLen=0;g_TextBufferLen<TEXTBUFFER_SIZE;g_TextBufferLen++)
	{
		g_TextBuffer[g_TextBufferLen] = eeprom_read_byte( szText + g_TextBufferLen ); //read byte from eeprom
		if( 0 == g_TextBuffer[g_TextBufferLen] || 0xFF /*EEPROM not programmed*/ == g_TextBuffer[g_TextBufferLen] )
		{
			break; //end of string
		}
	}

	//scroll once entire text
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

	return BUTTON_NONE;
}

/**
 * Displays animation of check mark.
 */
void AnimateCheck()
{
	for(uint8_t i=0;i<9;i++)
	{
		ledPutc(SYMBOL_CHECK_MARK + i);
		_delay_ms(20);
	}

	_delay_ms(200);
}

/**
 * \brief Displays light level on LED.
 * Used for self-test.
 *
 * Press button to exit the test
 */
void BrightnessLevel()
{
	uint8_t level;
	uint8_t button;

	do
	{
		button = ButtonCheck();
		level = GetLight();

		level /= 32; //convert to 8 levels

		/* This displays digit from 0 to 7
		ledPutc('0'+level); */


		/* This is a test code to display ADC value on LED instead of bar graph
		 * Don't forget to comment out "level /= 32;" line
		sprintf(g_TextBuffer," %d", level);
		g_TextBufferLen = strlen( g_TextBuffer );
		ShowText();
		*/


		ledPutc(SYMBOL_LIGHT_LEVEL + 7-level);

		_delay_ms( 20 );
	} while( button != BUTTON_DOWN );
}

/**
 * \name Gear animation
 * @{
 */

/**
 * \brief Animate gear number horizontally.
 *
 * \param prev Previous gear number
 * \param gear Current gear number
 *
 * \sa AnimateVertical() ShiftLeft() ShiftRight()
 */

void AnimateHorizontal(uint8_t prev, uint8_t gear)
{
	prev += SYMBOL_GEAR_NUMBER;
	gear += SYMBOL_GEAR_NUMBER;

	for(uint8_t i=0;i<=8;i++)
	{
		if( gear > prev )
			ShiftLeft(i, (PGM_P)FONTTAB+prev*8, (PGM_P)FONTTAB+gear*8 );
		else
			ShiftRight(i, (PGM_P)FONTTAB+prev*8, (PGM_P)FONTTAB+gear*8 );

		_delay_ms(GEAR_ANIM_DELAY);
	}

	ledPutc(gear);
}

/**
 * \brief Animate gear number vertically.
 *
 * \param prev Previous gear number
 * \param gear Current gear number
 *
 * \sa AnimateHorizontal() ShiftUp() ShiftDown()
 */

void AnimateVertical(uint8_t prev, uint8_t gear)
{
	//convert to ASCII codes
	prev += SYMBOL_GEAR_NUMBER;
	gear += SYMBOL_GEAR_NUMBER;

	for(uint8_t i=0;i<8;i++)
	{
		if( gear < prev )
			ShiftUp(i, (PGM_P)FONTTAB+gear*8, (PGM_P)FONTTAB+prev*8 );
		else
			ShiftDown(8-i, (PGM_P)FONTTAB+prev*8, (PGM_P)FONTTAB+gear*8  );

		_delay_ms(GEAR_ANIM_DELAY);
	}
	ledPutc(gear);
}

/**
 * Displays gear number of LED display.
 * \param gear Current gear number.
 *
 * \sa AnimateVertical() AnimateHorizontal()
 */
void AnimateNone( uint8_t gear )
{
	ledPutc(32+gear);
}

/**
 * \brief  Animate gear number.
 * Type of animation is taken from \a g_Config.GearAnimation.
 *
 * \param prev Previous gear number
 * \param gear Current gear number
 *
 * \sa AnimateVertical() AnimateHorizontal() AnimateNone()
 */
void Animate( uint8_t prev, uint8_t gear )
{
	switch( g_Config.GearAnimation )
	{
	default:
	case CONF_ANIM_UPDOWN:
		AnimateVertical(prev, gear);
		break;

	case CONF_ANIM_LEFTRIGHT:
		AnimateHorizontal(prev, gear);
		break;

	case CONF_ANIM_NONE:
		AnimateNone(gear);
		break;
	}
}

///@}

/**
 * \brief Display interrupt.
 *
 * The interrupt routine is called every time timer0 overflows.
 * First data from \ref DisplayBuffer is copied to \ref HardwareBuffer, then
 * columns and rows are set.
 *
 * At time only 8 pixels (row) are on.
 *
 * Automatic brightness is handles here. There are 16 levels of brightness.
 *
 * \sa InitializeHardware()
 */
ISR(TIMER0_OVF_vect)
{
	static uint8_t row=0;
	static uint8_t pwm=0;

	PORTB = 0; //disable display
	PORTD = 0;

	//an extra cycle to copy data and read light sensor
	if( row >= 8 )
	{
		switch( g_Config.DisplayRotation )
		{
		default:
		case 0: //0deg
			CopyDisplayToHardware0();
			break;

		case 1: //90deg
			CopyDisplayToHardware90();
			break;

		case 2: //180deg
			CopyDisplayToHardware180();
			break;

		case 3: //270deg
			CopyDisplayToHardware270();
			break;
		}

		g_LedBrightness = g_Config.fAutoBrightnessOff ? BRIGHTNESS_MAX : 1-GetLight()/16;

		if( g_LedBrightness < g_Config.MinBrightness*3 )//3 -> gives maximum "minimal" brightness 3*3 = 9 (max 15)
		{
			g_LedBrightness = g_Config.MinBrightness*3;
		}

		row = 0;
		pwm = 0;

		return;
	}

	PORTB = 1 << row;

	if( pwm < (g_LedBrightness % 16) /*brightness level 0-15*/)
	{
		PORTD = HardwareBuffer[ row ];
	}
	else
	{
		PORTD = 0;
	}

	if( pwm++ >= 8 )
	{
		pwm = 0;
		row++;
	}
}

/** @} */
