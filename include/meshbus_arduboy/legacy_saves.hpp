/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <meshbus_arduboy/save_format.hpp>
namespace meshbus::arduboy::legacy_saves {
using namespace meshbus::arduboy;
// Explicit opt-in for the pinned Hopper/Hollow ports, not an arbitrary raw decoder.
inline int migrate_obono(uint32_t schema, const uint8_t *src, size_t n,
                         uint8_t *dst, size_t size, size_t base, uint32_t signature)
{
    if (schema != 0 || n != eeprom_capacity || size != eeprom_capacity) { return -EINVAL; }
    bool erased = true;
    for (size_t i=base; i<base+32; ++i) { erased &= src[i] == 0xff; }
    if (!erased) {
        if (save_format::read32(src + base) != signature) { return -EBADMSG; }
        uint16_t sum = static_cast<uint16_t>(signature + (signature >> 16) * 2);
        for (unsigned i=0; i<13; ++i) {
            sum = static_cast<uint16_t>(sum + save_format::read16(src + base + 4 + i*2) * (i+3));
        }
        if (sum != save_format::read16(src + base + 30)) { return -EBADMSG; }
    }
    memcpy(dst, src, size);
    memset(dst, 0xff, eeprom_game_start);
    dst[eeprom_audio_setting] = src[0] ? 1 : 0;
    return 0;
}
inline int hopper(uint32_t schema, const uint8_t *src, size_t n, uint8_t *dst, size_t size)
{
    return migrate_obono(schema, src, n, dst, size, 800, UINT32_C(0x024e424f));
}
inline int hollow(uint32_t schema, const uint8_t *src, size_t n, uint8_t *dst, size_t size)
{
    return migrate_obono(schema, src, n, dst, size, 768, UINT32_C(0x014e424f));
}
}
