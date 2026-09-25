/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_PGMSPACE_HPP_
#define MESHBUS_ARDUBOY_PGMSPACE_HPP_
#include <stdint.h>
#include <string.h>

/* AVR numeric resources are little endian; their address need not be aligned. */
static inline uint8_t pgm_read_byte(const void *address)
{
    return *static_cast<const uint8_t *>(address);
}
static inline uint16_t pgm_read_word(const void *address)
{
    const auto *p = static_cast<const uint8_t *>(address);
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}
static inline uint32_t pgm_read_dword(const void *address)
{
    const auto *p = static_cast<const uint8_t *>(address);
    return static_cast<uint32_t>(pgm_read_word(p)) |
           (static_cast<uint32_t>(pgm_read_word(p + 2)) << 16);
}
/* Pointer tables are compiled for the target, never serialized AVR addresses. */
static inline void *pgm_read_ptr(const void *address)
{
    void *value;
    memcpy(&value, address, sizeof(value));
    return value;
}
template <typename T> static inline T *pgm_read_ptr(T *const *address)
{
    T *value;
    memcpy(&value, address, sizeof(value));
    return value;
}
#endif
