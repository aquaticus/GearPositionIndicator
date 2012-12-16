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
 * @brief Button functions header
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>

/**
 * Button port
 */
#define BUTTON_PORT PINC

/**
 * Button pin number
 */
#define BUTTON_PIN 1

/**
 * Time for button state to be stable in milliseconds.
 */
#define BUTTON_DEBOUNCE_TIME 20

/**
 * @name Results user interface
 * @{
 */
/// Nothing happened
#define BUTTON_NONE  0
/// Short click
#define BUTTON_SHORT 1
/// Long click
#define BUTTON_LONG  2
/// Detected change in button state.
#define BUTTON_DOWN 3
/// Indicates switch to temperature only display mode.
#define BUTTON_TEMP_MODE 4
/// Indicates switch to gear only display mode.
#define BUTTON_GEAR_MODE 5
///@}


uint8_t ButtonDown();
uint8_t ButtonCheck();
void ResetButtonState();

#endif /* BUTTON_H_ */
