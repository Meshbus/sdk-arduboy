/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_ARDUINO_SPI_H_
#define MESHBUS_ARDUBOY_ARDUINO_SPI_H_

#include <Arduino.h>

#define SPI_CLOCK_DIV2 2

class SPIClass {
public:
	void begin()
	{
        MESHBUS_ARDUBOY_UNSUPPORTED(SPI);
	}

	void setClockDivider(uint8_t divider)
	{
		MESHBUS_ARDUBOY_UNSUPPORTED(SPI);
		(void)divider;
	}

	uint8_t transfer(uint8_t value)
	{
		MESHBUS_ARDUBOY_UNSUPPORTED(SPI);
		return value;
	}
};

static SPIClass SPI;

#endif /* MESHBUS_ARDUBOY_ARDUINO_SPI_H_ */
