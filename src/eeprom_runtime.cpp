/* SPDX-License-Identifier: Apache-2.0 */
#include <meshbus_arduboy/eeprom_runtime.hpp>
#include <zephyr/sys/printk.h>
namespace meshbus::arduboy::storage_detail {
uint8_t scratch[eeprom_capacity + save_format::header_size];
unsigned scratch_busy;
}
namespace meshbus::arduboy {

namespace eeprom_runtime_detail {
static EepromFile *active_eeprom;
}

EepromFile *active_eeprom()
{
	#ifdef MESHBUS_ARDUBOY_RUNTIME
    if (eeprom_runtime_detail::active_eeprom == nullptr) {
        printk("[arduboy] EEPROM unavailable outside a bound session\n");
    }
#endif
    return eeprom_runtime_detail::active_eeprom;
}

void eeprom_attach(EepromFile *eeprom)
{
	eeprom_runtime_detail::active_eeprom = eeprom;
}

void eeprom_detach(EepromFile *eeprom)
{
	if (eeprom_runtime_detail::active_eeprom == eeprom) {
		eeprom_runtime_detail::active_eeprom = nullptr;
	}
}

} /* namespace meshbus::arduboy */

extern "C" size_t meshbus_arduboy_eeprom_length()
{
	meshbus::arduboy::EepromFile *eeprom = meshbus::arduboy::active_eeprom();

	return eeprom == nullptr ? 0U : eeprom->size();
}

extern "C" uint8_t meshbus_arduboy_eeprom_read(size_t address)
{
	meshbus::arduboy::EepromFile *eeprom = meshbus::arduboy::active_eeprom();

	return eeprom == nullptr ? 0xffU : eeprom->read(address);
}

extern "C" void meshbus_arduboy_eeprom_write(size_t address, uint8_t value)
{
	meshbus::arduboy::EepromFile *eeprom = meshbus::arduboy::active_eeprom();

	if (eeprom != nullptr) {
		eeprom->write(address, value);
	}
}

extern "C" void meshbus_arduboy_eeprom_update(size_t address, uint8_t value)
{
	meshbus::arduboy::EepromFile *eeprom = meshbus::arduboy::active_eeprom();

	if (eeprom != nullptr) {
		eeprom->update(address, value);
	}
}

extern "C" void meshbus_arduboy_eeprom_read_block(void *dst, size_t address, size_t len)
{
	meshbus::arduboy::EepromFile *eeprom = meshbus::arduboy::active_eeprom();

	if (dst == nullptr) {
		return;
	}
	if (eeprom == nullptr) {
		memset(dst, 0xff, len);
		return;
	}
	eeprom->read_block(dst, address, len);
}

extern "C" void meshbus_arduboy_eeprom_write_block(const void *src, size_t address, size_t len)
{
	meshbus::arduboy::EepromFile *eeprom = meshbus::arduboy::active_eeprom();

	if (eeprom != nullptr) {
		eeprom->write_block(address, src, len);
	}
}

extern "C" void meshbus_arduboy_eeprom_update_block(const void *src, size_t address, size_t len)
{
	meshbus::arduboy::EepromFile *eeprom = meshbus::arduboy::active_eeprom();

	if (eeprom != nullptr) {
		eeprom->update_block(address, src, len);
	}
}
