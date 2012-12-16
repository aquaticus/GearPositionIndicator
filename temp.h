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
 * @brief Temperature functions header
 * @ingroup temp
 * @{
 */

#ifndef TEMP_H_
#define TEMP_H_

/**
 * @brief Invalid temperature value.
 */
#define INVALID_TEMP 9999

extern int16_t g_nTemperature;

/* One wire */

#define OW_PIN  PC5
#define OW_IN   PINC
#define OW_OUT  PORTC
#define OW_DDR  DDRC
#define OW_CONF_DELAYOFFSET 0
#define OW_RECOVERY_TIME 10 /* usec */

/**
 * \name 1wire low-level routines
 * @{
 */
#define OW_GET_IN()   ( OW_IN & (1<<OW_PIN))
#define OW_OUT_LOW()  ( OW_OUT &= (~(1 << OW_PIN)) )
#define OW_OUT_HIGH() ( OW_OUT |= (1 << OW_PIN) )
#define OW_DIR_IN()   ( OW_DDR &= (~(1 << OW_PIN )) )
#define OW_DIR_OUT()  ( OW_DDR |= (1 << OW_PIN) )
///@}

/**
 * \name 1wire commands
 * @{
 */
#define OW_SKIP_ROM     0xCC
#define OW_CONVERT      0x44
#define OW_READ         0xBE
///@}

uint8_t ow_reset(void);
uint8_t ow_bit_io(uint8_t b);
uint8_t ow_byte_wr(uint8_t b);
uint8_t ow_byte_rd( void );

uint8_t StartTempConversion();
uint8_t GetTempConversionResult();
int FormatTemperature();

///@}

#endif /* TEMP_H_ */
