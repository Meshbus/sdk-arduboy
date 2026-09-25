/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_FX_DATA_HPP_
#define MESHBUS_ARDUBOY_FX_DATA_HPP_
#include <meshbus_arduboy/resources.hpp>
#include <limits.h>

namespace meshbus::arduboy {
/** FXPK1 logical address space. One 256-byte decompression cache, no heap. */
class FxData {
public:
    int begin(uint16_t page)
    {
        error_ = 0; cached_page_ = UINT32_MAX; reads = decodes = 0;
        uint8_t header[20];
        int rc = resource_read(0, header, sizeof(header));
        if (rc) { return fail(rc); }
        if (memcmp(header, "FXPK", 4) || save_format::read16(header + 4) != 1 ||
            save_format::read16(header + 6) != 256 || save_format::read16(header + 12) != page ||
            save_format::read16(header + 14)) { return fail(-EPROTONOSUPPORT); }
        length_ = save_format::read32(header + 8);
        uint32_t blocks = save_format::read32(header + 16);
        if (!length_ || length_ > 0x1000000U || blocks != (length_ + 255U) / 256U) {
            return fail(-EBADMSG);
        }
        uint32_t previous = 20U + (blocks + 1U) * 4U;
        for (uint32_t i = 0; i <= blocks; ++i) {
            uint32_t offset;
            rc = index(i, offset);
            if (rc) { return fail(rc); }
            if (offset > resource_length() || (i == 0 ? offset != previous :
                offset <= previous || offset - previous > 512U)) { return fail(-EBADMSG); }
            previous = offset;
        }
        if (previous != resource_length()) { return fail(-EBADMSG); }
        return 0;
    }
    int read(uint32_t offset, void *destination, size_t count)
    {
        if (error_) { return error_; }
        if ((!destination && count) || offset > length_ || count > length_ - offset) {
            return fail(-ERANGE);
        }
        ++reads;
        auto *out = static_cast<uint8_t *>(destination);
        while (count) {
            uint32_t page = offset / 256U;
            if (page != cached_page_) {
                int rc = decode(page);
                if (rc) { return fail(rc); }
            }
            size_t take = 256U - offset % 256U;
            if (take > count) { take = count; }
            memcpy(out, cache_ + offset % 256U, take);
            out += take; offset += take; count -= take;
        }
        return 0;
    }
    int error() const { return error_; }
    uint32_t length() const { return error_ ? 0 : length_; }
    uint32_t reads = 0, decodes = 0;
private:
    int fail(int error) { error_ = error; cached_page_ = UINT32_MAX; return error; }
    int index(uint32_t page, uint32_t &value)
    {
        uint8_t bytes[4]; int rc = resource_read(20U + page * 4U, bytes, sizeof(bytes));
        if (rc == 0) { value = save_format::read32(bytes); }
        return rc;
    }
    int decode(uint32_t page)
    {
        uint32_t position, end;
        int rc = index(page, position);
        if (rc == 0) { rc = index(page + 1U, end); }
        if (rc) { return rc; }
        unsigned produced = 0;
        unsigned wanted = length_ - page * 256U;
        if (wanted > 256U) { wanted = 256U; }
        while (position < end && produced < wanted) {
            uint8_t control;
            rc = resource_read(position++, &control, 1);
            if (rc) { return rc; }
            unsigned count = control < 128 ? control + 1U : (control & 127U) + 3U;
            if (count > wanted - produced) { return -EBADMSG; }
            if (control < 128) {
                if (count > end - position) { return -EBADMSG; }
                rc = resource_read(position, cache_ + produced, count);
                if (rc) { return rc; }
                position += count; produced += count;
            } else {
                uint8_t encoded_distance;
                if (position >= end) { return -EBADMSG; }
                rc = resource_read(position++, &encoded_distance, 1);
                if (rc) { return rc; }
                unsigned distance = encoded_distance ? encoded_distance : 256U;
                if (distance > produced) { return -EBADMSG; }
                for (unsigned i = 0; i < count; ++i, ++produced) {
                    cache_[produced] = cache_[produced - distance];
                }
            }
        }
        if (produced != wanted || position != end) { return -EBADMSG; }
        ++decodes; cached_page_ = page;
        return 0;
    }
    uint8_t cache_[256]{};
    uint32_t length_ = 0, cached_page_ = UINT32_MAX;
    int error_ = -ENODEV;
};
}
#endif
