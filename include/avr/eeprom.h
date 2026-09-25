/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_AVR_EEPROM_H_
#define MESHBUS_ARDUBOY_AVR_EEPROM_H_

#include <stddef.h>
#include <stdint.h>

#include <EEPROM.h>

static inline size_t meshbus_arduboy_avr_eeprom_addr(const void *addr)
{
	return reinterpret_cast<uintptr_t>(addr);
}

static inline void eeprom_busy_wait()
{
}

static inline uint8_t eeprom_read_byte(const uint8_t *addr)
{
	return meshbus_arduboy_eeprom_read(meshbus_arduboy_avr_eeprom_addr(addr));
}

static inline uint16_t eeprom_read_word(const uint16_t *addr)
{
	size_t offset = meshbus_arduboy_avr_eeprom_addr(addr);

	return static_cast<uint16_t>(meshbus_arduboy_eeprom_read(offset)) |
	       (static_cast<uint16_t>(meshbus_arduboy_eeprom_read(offset + 1U)) << 8);
}

static inline uint32_t eeprom_read_dword(const uint32_t *addr)
{
	size_t offset = meshbus_arduboy_avr_eeprom_addr(addr);

	return static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset)) |
	       (static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset + 1U)) << 8) |
	       (static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset + 2U)) << 16) |
	       (static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset + 3U)) << 24);
}

static inline void eeprom_read_block(void *dst, const void *addr, size_t len)
{
	meshbus_arduboy_eeprom_read_block(dst, meshbus_arduboy_avr_eeprom_addr(addr), len);
}

static inline void eeprom_write_byte(uint8_t *addr, uint8_t value)
{
	meshbus_arduboy_eeprom_write(meshbus_arduboy_avr_eeprom_addr(addr), value);
}

static inline void eeprom_write_word(uint16_t *addr, uint16_t value)
{
	size_t offset = meshbus_arduboy_avr_eeprom_addr(addr);

	meshbus_arduboy_eeprom_write(offset, static_cast<uint8_t>(value));
	meshbus_arduboy_eeprom_write(offset + 1U, static_cast<uint8_t>(value >> 8));
}

static inline void eeprom_write_dword(uint32_t *addr, uint32_t value)
{
	size_t offset = meshbus_arduboy_avr_eeprom_addr(addr);

	for (size_t i = 0U; i < 4U; i++) {
		meshbus_arduboy_eeprom_write(offset + i, static_cast<uint8_t>(value >> (i * 8U)));
	}
}

static inline void eeprom_write_block(const void *src, void *addr, size_t len)
{
	meshbus_arduboy_eeprom_write_block(src, meshbus_arduboy_avr_eeprom_addr(addr), len);
}

static inline void eeprom_update_byte(uint8_t *addr, uint8_t value)
{
	meshbus_arduboy_eeprom_update(meshbus_arduboy_avr_eeprom_addr(addr), value);
}

static inline void eeprom_update_word(uint16_t *addr, uint16_t value)
{
	size_t offset = meshbus_arduboy_avr_eeprom_addr(addr);

	meshbus_arduboy_eeprom_update(offset, static_cast<uint8_t>(value));
	meshbus_arduboy_eeprom_update(offset + 1U, static_cast<uint8_t>(value >> 8));
}

static inline void eeprom_update_dword(uint32_t *addr, uint32_t value)
{
	size_t offset = meshbus_arduboy_avr_eeprom_addr(addr);

	for (size_t i = 0U; i < 4U; i++) {
		meshbus_arduboy_eeprom_update(offset + i,
					   static_cast<uint8_t>(value >> (i * 8U)));
	}
}

static inline void eeprom_update_block(const void *src, void *addr, size_t len)
{
	meshbus_arduboy_eeprom_update_block(src, meshbus_arduboy_avr_eeprom_addr(addr), len);
}

#endif /* MESHBUS_ARDUBOY_AVR_EEPROM_H_ */
