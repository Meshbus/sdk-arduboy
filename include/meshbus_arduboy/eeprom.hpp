/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_EEPROM_HPP_
#define MESHBUS_ARDUBOY_EEPROM_HPP_

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>

#include <meshbus_arduboy/save_store.hpp>

namespace meshbus::arduboy {

class EepromFile {
public:
	EepromFile() { k_sem_init(&lock_, 1, 1); }
	EepromFile(const EepromFile &) = delete;
	EepromFile &operator=(const EepromFile &) = delete;
	// init/reset/load require a stopped game; one owner calls flush.
	int init(const char *save_id, uint8_t *data, size_t size, uint8_t erased_value = 0xffU)
	{
        return init_impl(save_id, data, size, 0, nullptr, erased_value);
    }
    int init_versioned(const char *save_id, uint8_t *data, size_t size, uint32_t schema,
                       SaveMigration migration = nullptr, uint8_t erased_value = 0xffU)
    {
        if (!schema) { valid_ = false; return -EINVAL; }
        return init_impl(save_id, data, size, schema, migration, erased_value);
    }
    SaveLoadStatus load_status() const { return store_.status(); }

	void reset()
	{
		if (data_ != nullptr && size_ != 0U) {
			memset(data_, erased_value_, size_);
		}
		dirty_ = false;
	}

	int load()
	{
        reset();
        if (!ready()) { return -EINVAL; }
        return store_.load(data_, size_);
    }

	int flush()
	{
        if (!ready()) { return -EINVAL; }
        if (!store_.writable()) { return -EACCES; }
        if (!dirty()) { return 0; }
        storage_detail::ScratchLease lease;
        if (!lease.owned) { return -EBUSY; }
        auto *snapshot = storage_detail::scratch;
        uint32_t generation;
        {
            Guard guard(lock_);
            memcpy(snapshot, data_, size_);
            generation = generation_;
        }
        int rc = store_.commit(snapshot, size_);
        if (rc == 0) {
            Guard guard(lock_);
            dirty_ = generation_ != generation;
        }
        return rc;
	}

	uint8_t read(size_t address) const
	{
        Guard guard(lock_);
		if (data_ == nullptr || address >= size_) {
			return erased_value_;
		}
		return data_[address];
	}

	void write(size_t address, uint8_t value)
	{
        Guard guard(lock_);
		if (data_ == nullptr || address >= size_) {
			return;
		}
		data_[address] = value;
		dirty_ = true;
        ++generation_;
	}

	void update(size_t address, uint8_t value)
	{
        Guard guard(lock_);
		if (data_ == nullptr || address >= size_ || data_[address] == value) {
			return;
		}
		data_[address] = value;
		dirty_ = true;
        ++generation_;
	}

	void read_block(void *dst, size_t address, size_t len) const
    {
        if (dst == nullptr) { return; }
        Guard guard(lock_);
        auto *out = static_cast<uint8_t *>(dst);
        for (size_t i = 0; i < len; ++i) {
            out[i] = data_ != nullptr && address < size_ && i < size_ - address
                         ? data_[address + i] : erased_value_;
        }
    }
    void write_block(size_t address, const void *src, size_t len)
    {
        update_block(address, src, len);
    }
    void update_block(size_t address, const void *src, size_t len)
    {
        if (src == nullptr || data_ == nullptr || address >= size_) { return; }
        Guard guard(lock_);
        const auto *in = static_cast<const uint8_t *>(src);
        size_t count = len < size_ - address ? len : size_ - address;
        if (memcmp(data_ + address, in, count) != 0) {
            memcpy(data_ + address, in, count);
            ++generation_;
            dirty_ = true;
        }
    }

	bool dirty() const
	{
        Guard guard(lock_);
		return dirty_;
	}

	size_t size() const
	{
		return size_;
	}

	const char *path() const
	{
		return store_.path();
	}

private:
    struct Guard {
        k_sem &lock;
        explicit Guard(k_sem &value) : lock(value) { k_sem_take(&lock, K_FOREVER); }
        ~Guard() { k_sem_give(&lock); }
    };
    mutable k_sem lock_;
    uint32_t generation_ = 0;
	bool ready() const
	{
		return valid_ && data_ != nullptr && size_ != 0U;
	}

    SaveStore store_;
    bool valid_ = false;
    int init_impl(const char *id, uint8_t *data, size_t size, uint32_t schema,
                  SaveMigration migration, uint8_t erased)
    {
        valid_ = false;
        data_ = nullptr; size_ = 0;
        if (!data || !size) { return -EINVAL; }
        if (size > eeprom_capacity) { return -E2BIG; }
        int rc = store_.init(id, schema, migration);
        if (rc) { return rc; }
        data_ = data; size_ = size; erased_value_ = erased;
        generation_ = 0; reset(); valid_ = true;
        return 0;
    }
	uint8_t *data_ = nullptr;
	size_t size_ = 0U;
	uint8_t erased_value_ = 0xffU;
	bool dirty_ = false;
};

} /* namespace meshbus::arduboy */

#endif /* MESHBUS_ARDUBOY_EEPROM_HPP_ */
