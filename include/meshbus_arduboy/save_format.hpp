/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_SAVE_FORMAT_HPP_
#define MESHBUS_ARDUBOY_SAVE_FORMAT_HPP_
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace meshbus::arduboy {
constexpr size_t eeprom_capacity = 1024;
constexpr size_t eeprom_game_start = 16;
constexpr size_t eeprom_audio_setting = 2;
using SaveMigration = int (*)(uint32_t from_schema, const uint8_t *source, size_t source_size,
                             uint8_t *destination, size_t destination_size);
enum class SaveLoadStatus {
    not_loaded, fresh, loaded, legacy_available, migrated, corrupt,
    size_mismatch, schema_mismatch, identity_mismatch, migration_failed, io_error,
};
namespace save_format {
constexpr size_t header_size = 80;
constexpr size_t identity_size = 48;
inline uint16_t read16(const uint8_t *p)
{
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}
inline uint32_t read32(const uint8_t *p)
{
    return uint32_t(read16(p)) | (uint32_t(read16(p + 2)) << 16);
}
inline void write16(uint8_t *p, uint16_t value)
{
    p[0] = static_cast<uint8_t>(value); p[1] = static_cast<uint8_t>(value >> 8);
}
inline void write32(uint8_t *p, uint32_t value)
{
    write16(p, static_cast<uint16_t>(value)); write16(p + 2, static_cast<uint16_t>(value >> 16));
}
inline uint32_t crc32(const uint8_t *data, size_t size)
{
    uint32_t crc = UINT32_MAX;
    for (size_t i=0; i<size; ++i) {
        crc ^= data[i];
        for (unsigned bit=0; bit<8; ++bit) {
            crc = (crc >> 1) ^ ((0U - (crc & 1U)) & UINT32_C(0xedb88320));
        }
    }
    return ~crc;
}
inline int encode(uint8_t *header, const char *identity, uint32_t schema,
                  const uint8_t *data, size_t size)
{
    if (!header || !identity || !data || !schema || !size || size > eeprom_capacity ||
        strlen(identity) >= identity_size) { return -EINVAL; }
    memset(header, 0, header_size);
    memcpy(header, "MASV", 4);
    write16(header + 4, 1); write16(header + 6, header_size);
    write32(header + 8, schema); write32(header + 12, size);
    write32(header + 16, crc32(data, size));
    memcpy(header + 24, identity, strlen(identity) + 1);
    write32(header + 72, crc32(header, 72));
    return 0;
}
struct Header { uint32_t schema, payload_size; };
inline int decode(const uint8_t *file, size_t size, const char *identity, Header &header)
{
    if (!file || !identity || size < header_size) { return -EBADMSG; }
    if (memcmp(file, "MASV", 4) != 0) { return -EBADMSG; }
    if (read16(file + 4) != 1 || read16(file + 6) != header_size ||
        read32(file + 20) != 0 || read32(file + 76) != 0) { return -EPROTONOSUPPORT; }
    if (read32(file + 72) != crc32(file, 72)) { return -EBADMSG; }
    size_t id_size = strlen(identity);
    if (id_size >= identity_size || memcmp(file + 24, identity, id_size + 1) != 0) {
        return -EXDEV;
    }
    for (size_t i=id_size + 1; i<identity_size; ++i) {
        if (file[24 + i] != 0) { return -EBADMSG; }
    }
    header = {read32(file + 8), read32(file + 12)};
    if (!header.schema || !header.payload_size || header.payload_size > eeprom_capacity ||
        size - header_size != header.payload_size) { return -EMSGSIZE; }
    if (read32(file + 16) != crc32(file + header_size, header.payload_size)) { return -EBADMSG; }
    return 0;
}
} // namespace save_format
} // namespace meshbus::arduboy
#endif
