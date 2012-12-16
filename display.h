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
 * @brief LED functions
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include <avr/pgmspace.h>

/**
 * \name Symbols in FONTTAB
 * ASCII codes
 * @{
 */

/**
 * Degrees symbol.
 */
#define SYMBOL_DEG 28

/**
 * Left arrow symbol
 */
#define SYMBOL_ARROW 26

/**
 * ASCII code of the first character used to display light level.
 */
#define SYMBOL_LIGHT_LEVEL 1

/**
 * ASCII code of the first gear number character.
 */
#define SYMBOL_GEAR_NUMBER 9

#define SYMBOL_CHECK_MARK 16

///@}

/**
 * Maximum brightness level
 */
#define BRIGHTNESS_MAX (16-1)

/**
 * Maximum size of \a g_TextBuffer;
 */
#define TEXTBUFFER_SIZE 64

/**
 * @brief Animation delay in ms.
 * Affects gear animation speed.
 */
#define GEAR_ANIM_DELAY 20


void ledPutc( char c );
void ledNegPutc( char c );

void ScrollDelay();

extern char g_TextBuffer[];
extern uint8_t g_TextBufferLen;
extern PGM_VOID_P pCurrentFont;

/// Alphanumeric symbols, 8x8 pixels.
extern PROGMEM unsigned char FONTTAB[];

/// Symbols for light level indicator.
extern unsigned char PROGMEM LIGHTLEVEL[];

/// Gear numbers.
extern unsigned char PROGMEM GEARS[];

/// Check symbol animation.
extern unsigned char PROGMEM CHECK[];

extern volatile uint8_t DisplayBuffer[8];

void Animate( uint8_t prev, uint8_t gear );

void DisplayTemperature();

uint8_t GetLight();

void ScrollDelay();

void ShiftLeft( uint8_t offset, PGM_P OldData, PGM_P NewData );
void ShiftRight( uint8_t offset, PGM_P OldData, PGM_P NewData );
void ShiftUp( uint8_t offset, PGM_P OldData, PGM_P NewData );
void ShiftDown( uint8_t offset, PGM_P OldData, PGM_P NewData );

int ScrollLeft( const char* szText, int Len, int* pOffset );
int ScrollUp( const char* szText, int Len, int* pOffset );
int ScrollDown( const char* szText, int Len, int* pOffset );

void FlashChar(char c, uint8_t n);
void FlashCharNeg(char c, uint8_t n);
void AnimateCheck();
void BrightnessLevel();
uint8_t ledPuts_EE( const uint8_t* szText );

#endif /* DISPLAY_H_ */
