/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_SAVE_STORE_HPP_
#define MESHBUS_ARDUBOY_SAVE_STORE_HPP_
#include <meshbus_arduboy/storage.hpp>
#include <meshbus_arduboy/save_format.hpp>
namespace meshbus::arduboy {
namespace storage_detail {
extern uint8_t scratch[eeprom_capacity + save_format::header_size];
extern unsigned scratch_busy;
struct ScratchLease {
    bool owned = !__atomic_exchange_n(&scratch_busy, 1U, __ATOMIC_ACQUIRE);
    ~ScratchLease() { if (owned) { __atomic_store_n(&scratch_busy, 0U, __ATOMIC_RELEASE); } }
    ScratchLease() = default;
    ScratchLease(const ScratchLease &) = delete;
    ScratchLease &operator=(const ScratchLease &) = delete;
};
inline int read_save(const char *path, uint8_t *data, size_t capacity, size_t &size)
{
    fs_file_t file; fs_file_t_init(&file);
    int rc = fs_open(&file, path, FS_O_READ);
    if (rc) { return rc; }
    size = 0;
    while (size < capacity) {
        ssize_t n = fs_read(&file, data + size, capacity - size);
        if (n < 0) { rc = static_cast<int>(n); break; }
        if (!n) { break; }
        if (static_cast<size_t>(n) > capacity - size) { rc = -EIO; break; }
        size += static_cast<size_t>(n);
    }
    if (!rc && size == capacity) {
        uint8_t extra;
        ssize_t n = fs_read(&file, &extra, 1);
        if (n != 0) { rc = n < 0 ? static_cast<int>(n) : -EMSGSIZE; }
    }
    int close_rc = fs_close(&file);
    return rc ? rc : close_rc;
}
}
class SaveStore {
public:
    int init(const char *id, uint32_t schema = 0, SaveMigration migration = nullptr)
    {
        writable_ = false; status_ = SaveLoadStatus::not_loaded;
        path_[0] = 0; legacy_[0] = 0;
        int rc = make_save_path(legacy_, sizeof(legacy_), id);
        if (rc) { return rc; }
        if (strlen(id) >= sizeof(identity_)) { return -ENAMETOOLONG; }
        memcpy(identity_, id, strlen(id) + 1);
        memcpy(path_, legacy_, strlen(legacy_) + 1);
        schema_ = schema; migration_ = migration;
        if (schema) { memcpy(path_ + strlen(path_) - 4, ".sav", 5); }
        writable_ = !schema;
        return 0;
    }
    const char *path() const { return path_; }
    bool writable() const { return writable_; }
    SaveLoadStatus status() const { return status_; }
    int load(uint8_t *data, size_t size)
    {
        writable_ = false;
        storage_detail::ScratchLease lease;
        if (!lease.owned) { return fail(-EBUSY, SaveLoadStatus::io_error); }
        auto *source = storage_detail::scratch;
        size_t count = 0;
        int rc = storage_detail::read_save(path_, source, sizeof(storage_detail::scratch), count);
        bool legacy = false;
        if (rc == -ENOENT && schema_) {
            legacy = true;
            rc = storage_detail::read_save(legacy_, source, sizeof(storage_detail::scratch), count);
        }
        if (rc == -ENOENT) {
            status_ = SaveLoadStatus::fresh; writable_ = true; return 0;
        }
        if (rc) { return fail(rc, rc == -EMSGSIZE ? SaveLoadStatus::size_mismatch : SaveLoadStatus::io_error); }
        uint32_t source_schema = 0;
        if (schema_ && !legacy) {
            save_format::Header header;
            rc = save_format::decode(source, count, identity_, header);
            if (rc) {
                return fail(rc, rc == -EXDEV ? SaveLoadStatus::identity_mismatch :
                    rc == -EMSGSIZE ? SaveLoadStatus::size_mismatch :
                    rc == -EPROTONOSUPPORT ? SaveLoadStatus::schema_mismatch : SaveLoadStatus::corrupt);
            }
            source_schema = header.schema;
            source += save_format::header_size; count = header.payload_size;
        }
        if (source_schema == schema_ && count == size) {
            memcpy(data, source, size); writable_ = true;
            status_ = SaveLoadStatus::loaded; return 0;
        }
        if (!migration_) {
            return fail(legacy ? -EAGAIN : count != size ? -EMSGSIZE : -EPROTONOSUPPORT,
                legacy ? SaveLoadStatus::legacy_available : count != size ? SaveLoadStatus::size_mismatch : SaveLoadStatus::schema_mismatch);
        }
        rc = migration_(source_schema, source, count, data, size);
        if (rc) { return fail(rc, SaveLoadStatus::migration_failed); }
        rc = write_current(data, size);
        if (rc) { return fail(rc, SaveLoadStatus::io_error); }
        writable_ = true; status_ = SaveLoadStatus::migrated; return 0;
    }
    int commit(const uint8_t *data, size_t size)
    {
        return writable_ ? write_current(data, size) : -EACCES;
    }
private:
    int fail(int rc, SaveLoadStatus status) { status_ = status; return rc; }
    int write_current(const uint8_t *data, size_t size)
    {
        if (!schema_) { return storage_detail::commit_snapshot(path_, data, size); }
        uint8_t header[save_format::header_size];
        int rc = save_format::encode(header, identity_, schema_, data, size);
        return rc ? rc : storage_detail::commit_snapshot_parts(path_, header, sizeof(header), data, size);
    }
    char path_[save_path_capacity] = {}, legacy_[save_path_capacity] = {};
    char identity_[save_format::identity_size] = {};
    uint32_t schema_ = 0;
    SaveMigration migration_ = nullptr;
    bool writable_ = false;
    SaveLoadStatus status_ = SaveLoadStatus::not_loaded;
};
}
#endif
