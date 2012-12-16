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
 * @brief CRC8 implementation
 */

#include <stdint.h>

/**
 * Compute 8 bit CRC for 1wire protocol.
 * Polynomial is x^8+x^5+x^4+x^0, seed valued is \b 0.
 *
 * @param pData Pointer to data
 * @param len Length of the data
 * @return 8 bit checksum
 *
 * @par Example
 * "123456789" crc8 for 9 bytes is 0xA1.
 */
uint8_t crc8(uint8_t* pData, uint16_t len )
{
	const uint8_t poly = 0x18; //=x^8+x^5+x^4+x^0
	uint8_t crc=0; //seed = 0
	uint8_t b;

	for (uint16_t n = 0; n < len; n++)
	{
		b = pData[n];

		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (0x01 == ((crc ^ b) & 0x01))
			{
				crc = crc ^ poly;
				crc = (crc >> 1) & 0x7F;
				crc |= 0x80;
			}
			else
			{
				crc = (crc >> 1) & 0x7F;
			}

			b >>= 1;
		}
	}

	return crc;
}
