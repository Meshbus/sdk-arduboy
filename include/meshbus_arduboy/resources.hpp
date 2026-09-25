/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_RESOURCES_HPP_
#define MESHBUS_ARDUBOY_RESOURCES_HPP_
#include <meshbus_arduboy/save_format.hpp>
#include <zephyr/fs/fs.h>
#ifdef MESHBUS_ARDUBOY_RELOCATABLE_RESOURCES
#include <desktop/desktop.h>
#endif

namespace meshbus::arduboy {
struct ResourceSpec {
    const char *path;
    const char *identity;
    uint32_t version;
    uint32_t length;
    uint32_t crc32;
};

/** One application-thread owner. Never mutate the backing file while open.
 * The cache and scan buffer share 256 bytes. No payload-sized allocation.
 */
class ResourceBundle {
public:
    static constexpr uint32_t header_size = 64;
    static constexpr size_t cache_size = 256;
    ResourceBundle() = default;
    ResourceBundle(const ResourceBundle &) = delete;
    ResourceBundle &operator=(const ResourceBundle &) = delete;
    ~ResourceBundle() { close(); }

    int open(const ResourceSpec &spec)
    {
        if (opened_) { return -EBUSY; }
        if (!spec.path || !spec.identity || !spec.identity[0] || !spec.version || !spec.length ||
            strlen(spec.identity) > 31 || spec.length > INT32_MAX - header_size) {
            return -EINVAL;
        }
        fs_file_t_init(&file_);
#ifdef MESHBUS_ARDUBOY_RELOCATABLE_RESOURCES
        char installed[192];
        size_t name_offset = strlen(spec.path);
        while (name_offset > 0 && spec.path[name_offset - 1] != '/') { --name_offset; }
        int rc = mbs_desktop_app_resource_path(spec.path + name_offset, installed, sizeof(installed));
        if (rc != 0) { return rc; }
        rc = fs_open(&file_, installed, FS_O_READ);
#else
        int rc = fs_open(&file_, spec.path, FS_O_READ);
#endif
        if (rc != 0) { return rc; }
        opened_ = true;
        uint8_t header[header_size];
        rc = read_exact(header, sizeof(header));
        if (rc == 0) { rc = validate_header(header, spec); }
        uint32_t crc = UINT32_MAX;
        for (uint32_t done = 0; rc == 0 && done < spec.length;) {
            size_t count = spec.length - done;
            if (count > cache_size) { count = cache_size; }
            rc = read_exact(cache_, count);
            for (size_t i = 0; rc == 0 && i < count; ++i) {
                crc ^= cache_[i];
                for (unsigned bit = 0; bit < 8; ++bit) {
                    crc = (crc >> 1) ^ ((0U - (crc & 1U)) & UINT32_C(0xedb88320));
                }
            }
            done += count;
        }
        if (rc == 0 && ~crc != spec.crc32) { rc = -EBADMSG; }
        if (rc == 0) {
            ssize_t n = fs_read(&file_, cache_, 1);
            if (n != 0) { rc = n < 0 ? static_cast<int>(n) : -EFBIG; }
        }
        if (rc != 0) { close(); return rc; }
        length_ = spec.length;
        cached_ = 0;
        return 0;
    }

    int read(uint32_t offset, void *destination, size_t size)
    {
        if (!opened_) { return -ENODEV; }
        if ((!destination && size) || offset > length_ || size > length_ - offset) {
            return -ERANGE;
        }
        auto *out = static_cast<uint8_t *>(destination);
        while (size) {
            if (!cached_ || offset < start_ || offset - start_ >= cached_) {
                start_ = offset - offset % cache_size;
                cached_ = 0;
                int rc = fs_seek(&file_, header_size + start_, FS_SEEK_SET);
                if (rc != 0) { return rc; }
                size_t count = length_ - start_;
                if (count > cache_size) { count = cache_size; }
                rc = read_exact(cache_, count);
                if (rc != 0) { return rc; }
                cached_ = count;
            }
            size_t count = cached_ - (offset - start_);
            if (count > size) { count = size; }
            memcpy(out, cache_ + (offset - start_), count);
            out += count; offset += count; size -= count;
        }
        return 0;
    }
    uint32_t length() const { return opened_ ? length_ : 0; }
    int close()
    {
        if (!opened_) { return 0; }
        opened_ = false; cached_ = 0; length_ = 0;
        return fs_close(&file_);
    }

private:
    int read_exact(void *destination, size_t size)
    {
        auto *p = static_cast<uint8_t *>(destination);
        while (size) {
            ssize_t n = fs_read(&file_, p, size);
            if (n <= 0) { return n < 0 ? static_cast<int>(n) : -ENODATA; }
            if (static_cast<size_t>(n) > size) { return -EIO; }
            p += n; size -= n;
        }
        return 0;
    }
    static int validate_header(const uint8_t *h, const ResourceSpec &spec)
    {
        using namespace save_format;
        if (memcmp(h, "MARB", 4) || read32(h + 60) != crc32(h, 60)) { return -EBADMSG; }
        if (read16(h + 4) != 1 || read16(h + 6) != header_size ||
            read32(h + 20) || read32(h + 56)) { return -EPROTONOSUPPORT; }
        char identity[32]{};
        memcpy(identity, spec.identity, strlen(spec.identity));
        if (memcmp(h + 24, identity, sizeof(identity))) { return -EXDEV; }
        if (read32(h + 8) != spec.version) { return -EPROTONOSUPPORT; }
        if (read32(h + 12) != spec.length || read32(h + 16) != spec.crc32) { return -EBADMSG; }
        return 0;
    }
    fs_file_t file_{};
    uint8_t cache_[cache_size]{};
    uint32_t length_ = 0, start_ = 0;
    size_t cached_ = 0;
    bool opened_ = false;
};
// Runtime access belongs to the game thread and is valid only during run_sketch.
int resource_read(uint32_t offset, void *destination, size_t size);
uint32_t resource_length();
}
#endif
