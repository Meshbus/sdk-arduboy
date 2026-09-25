/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_ARDUINO_H_
#define MESHBUS_ARDUBOY_ARDUINO_H_

#include <meshbus_arduboy/capabilities.hpp>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

using byte = uint8_t;
using boolean = bool;

#ifndef PROGMEM
#define PROGMEM
#endif

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
static constexpr uint8_t A0 = 14;
static constexpr uint8_t A1 = 15;
static constexpr uint8_t A2 = 16;
static constexpr uint8_t A3 = 17;
#define RGB_ON LOW
#define RGB_OFF HIGH
#define _BV(bit) (1U << (bit))
#define bit_is_set(value, bit) (((value) & _BV(bit)) != 0U)

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

class __FlashStringHelper;

#ifndef F
#define F(str) (reinterpret_cast<const __FlashStringHelper *>(str))
#endif

#ifndef PSTR
#define PSTR(str) (str)
#endif

#ifndef strlen_P
#define strlen_P(str) strlen(str)
#endif

#ifndef strcpy_P
#define strcpy_P(dst, src) strcpy((dst), (src))
#endif

#ifndef memcpy_P
#define memcpy_P(dst, src, len) memcpy((dst), (src), (len))
#endif

extern "C" uint32_t meshbus_arduboy_millis();
extern "C" uint32_t meshbus_arduboy_micros();
extern "C" void meshbus_arduboy_random_seed(uint32_t seed);
extern "C" long meshbus_arduboy_random(long max);
extern "C" long meshbus_arduboy_random_range(long min, long max);

#include <meshbus_arduboy/pgmspace.hpp>

static inline uint32_t millis()
{
	return meshbus_arduboy_millis();
}

static inline uint32_t micros()
{
	return meshbus_arduboy_micros();
}

static inline void randomSeed(uint32_t seed)
{
	meshbus_arduboy_random_seed(seed);
}

static inline long random(long max)
{
	return meshbus_arduboy_random(max);
}

static inline long random(long min, long max)
{
	return meshbus_arduboy_random_range(min, max);
}

#ifdef MESHBUS_ARDUBOY_RUNTIME
extern "C" void meshbus_arduboy_delay(uint32_t milliseconds);
#endif
static inline void delay(uint32_t ms)
{
#ifdef MESHBUS_ARDUBOY_RUNTIME
    meshbus_arduboy_delay(ms);
#else
    MESHBUS_ARDUBOY_UNSUPPORTED(legacy_delay);
    (void)ms;
#endif
}

static inline void pinMode(uint8_t pin, uint8_t mode)
{
	MESHBUS_ARDUBOY_UNSUPPORTED(GPIO);
	(void)pin;
	(void)mode;
}

static inline void digitalWrite(uint8_t pin, uint8_t value)
{
	MESHBUS_ARDUBOY_UNSUPPORTED(GPIO);
	(void)pin;
	(void)value;
}

static inline void analogWrite(uint8_t pin, uint8_t value)
{
	MESHBUS_ARDUBOY_UNSUPPORTED(GPIO);
	(void)pin;
	(void)value;
}

#endif /* MESHBUS_ARDUBOY_ARDUINO_H_ */
