/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_EEPROM_RUNTIME_HPP_
#define MESHBUS_ARDUBOY_EEPROM_RUNTIME_HPP_
#include <meshbus_arduboy/eeprom.hpp>
namespace meshbus::arduboy {
EepromFile *active_eeprom();
void eeprom_attach(EepromFile *eeprom);
void eeprom_detach(EepromFile *eeprom);
}
extern "C" size_t meshbus_arduboy_eeprom_length();
extern "C" uint8_t meshbus_arduboy_eeprom_read(size_t address);
extern "C" void meshbus_arduboy_eeprom_write(size_t address, uint8_t value);
extern "C" void meshbus_arduboy_eeprom_update(size_t address, uint8_t value);
extern "C" void meshbus_arduboy_eeprom_read_block(void *dst, size_t address, size_t len);
extern "C" void meshbus_arduboy_eeprom_write_block(const void *src, size_t address, size_t len);
extern "C" void meshbus_arduboy_eeprom_update_block(const void *src, size_t address, size_t len);
#endif
