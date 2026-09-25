/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_ARDUINO_EEPROM_H_
#define MESHBUS_ARDUBOY_ARDUINO_EEPROM_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <type_traits>
#include <Arduino.h>

extern "C" size_t meshbus_arduboy_eeprom_length();
extern "C" uint8_t meshbus_arduboy_eeprom_read(size_t address);
extern "C" void meshbus_arduboy_eeprom_write(size_t address, uint8_t value);
extern "C" void meshbus_arduboy_eeprom_update(size_t address, uint8_t value);
extern "C" void meshbus_arduboy_eeprom_read_block(void *dst, size_t address, size_t len);
extern "C" void meshbus_arduboy_eeprom_write_block(const void *src, size_t address, size_t len);
extern "C" void meshbus_arduboy_eeprom_update_block(const void *src, size_t address, size_t len);

class EEPROMClass {
public:
	uint8_t read(int address) const
	{
		return address < 0 ? 0xffU : meshbus_arduboy_eeprom_read(static_cast<size_t>(address));
	}

	void write(int address, uint8_t value)
	{
		if (address >= 0) {
			meshbus_arduboy_eeprom_write(static_cast<size_t>(address), value);
		}
	}

	void update(int address, uint8_t value)
	{
		if (address >= 0) {
			meshbus_arduboy_eeprom_update(static_cast<size_t>(address), value);
		}
	}

	template <typename T> T &get(int address, T &value) const
	{
		static_assert(std::is_trivially_copyable<T>::value, "EEPROM objects must be trivially copyable");
        // Native layout only; use explicit fixed-width encoding for portable saves.
        if (address >= 0) {
			meshbus_arduboy_eeprom_read_block(&value, static_cast<size_t>(address),
						       sizeof(value));
		}
		return value;
	}

	template <typename T> const T &put(int address, const T &value)
	{
		static_assert(std::is_trivially_copyable<T>::value, "EEPROM objects must be trivially copyable");
        if (address >= 0) {
			meshbus_arduboy_eeprom_update_block(&value, static_cast<size_t>(address),
							 sizeof(value));
		}
		return value;
	}

	int length() const
	{
		return static_cast<int>(meshbus_arduboy_eeprom_length());
	}
};

static EEPROMClass EEPROM;

#endif /* MESHBUS_ARDUBOY_ARDUINO_EEPROM_H_ */
