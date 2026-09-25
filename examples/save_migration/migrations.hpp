/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <meshbus_arduboy/legacy_saves.hpp>
namespace save_examples {
using namespace meshbus::arduboy;
using legacy_saves::migrate_obono;
using legacy_saves::hopper;
using legacy_saves::hollow;
// Schema 1: AVR unsigned int score (u16), signed int position (i16), at byte 16.
// Schema 2: explicit LE u32/i32 fields at byte 16; no native object representation.
inline int avr_to_c2(uint32_t schema, const uint8_t *src, size_t n, uint8_t *dst, size_t size)
{
    if (schema != 1 || n != eeprom_capacity || size != eeprom_capacity) { return -EINVAL; }
    memcpy(dst, src, eeprom_game_start);
    save_format::write32(dst + 16, save_format::read16(src + 16));
    uint32_t position = save_format::read16(src + 18);
    if (position & 0x8000) { position |= UINT32_C(0xffff0000); }
    save_format::write32(dst + 20, position);
    return 0;
}
}
