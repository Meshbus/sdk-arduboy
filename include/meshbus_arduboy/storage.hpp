/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_STORAGE_HPP_
#define MESHBUS_ARDUBOY_STORAGE_HPP_

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/fs/fs.h>

namespace meshbus::arduboy {

static constexpr const char *save_dir = "/extra/saves";
static constexpr const char *save_ext = ".dat";
static constexpr size_t save_path_capacity = 64U;

static inline bool save_id_char_valid(char ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
	       (ch >= '0' && ch <= '9') || ch == '_' || ch == '-';
}

static inline bool save_id_valid(const char *save_id)
{
	if (save_id == nullptr || save_id[0] == '\0') {
		return false;
	}

	for (const char *p = save_id; *p != '\0'; p++) {
		if (!save_id_char_valid(*p)) {
			return false;
		}
	}

	return true;
}

static inline int make_save_path(char *dst, size_t dst_size, const char *save_id)
{
	const size_t dir_len = strlen(save_dir);
	const size_t id_len = save_id == nullptr ? 0U : strlen(save_id);
	const size_t ext_len = strlen(save_ext);
	const size_t total_len = dir_len + 1U + id_len + ext_len;
	char *out = dst;

	if (dst == nullptr || dst_size == 0U) {
		return -EINVAL;
	}
	dst[0] = '\0';

	if (!save_id_valid(save_id)) {
		return -EINVAL;
	}
	if (total_len >= dst_size) {
		return -ENAMETOOLONG;
	}

	memcpy(out, save_dir, dir_len);
	out += dir_len;
	*out++ = '/';
	memcpy(out, save_id, id_len);
	out += id_len;
	memcpy(out, save_ext, ext_len);
	out += ext_len;
	*out = '\0';

	return 0;
}

static inline int ensure_save_dir()
{
	int rc = fs_mkdir(save_dir);

	return rc == -EEXIST ? 0 : rc;
}

namespace storage_detail {

/* The caller owns an immutable snapshot until this function returns.
 * Commit requires the host filesystem to provide same-directory atomic rename.
 * Never truncate or unlink the previously committed save on an I/O failure.
 */
static inline int commit_snapshot_parts(const char *path, const uint8_t *prefix, size_t prefix_size,
                                        const uint8_t *data, size_t size)
{
    char temporary[save_path_capacity + sizeof(".tmp") - 1U];
    struct fs_file_t file;
    if (path == nullptr || data == nullptr || size == 0U ||
        strlen(path) >= save_path_capacity || (prefix_size && prefix == nullptr)) {
        return -EINVAL;
    }
    int rc = ensure_save_dir();
    if (rc != 0) {
        return rc;
    }
    const size_t length = strlen(path);
    memcpy(temporary, path, length);
    memcpy(temporary + length, ".tmp", sizeof(".tmp"));
    fs_file_t_init(&file);
    rc = fs_open(&file, temporary, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
    if (rc != 0) {
        return rc;
    }
    const uint8_t *parts[] = {prefix, data};
    const size_t lengths[] = {prefix_size, size};
    for (unsigned part=0; part<2 && rc == 0; ++part) {
        size_t offset = 0;
        while (offset < lengths[part]) {
            ssize_t written = fs_write(&file, parts[part] + offset, lengths[part] - offset);
            if (written <= 0 || static_cast<size_t>(written) > lengths[part] - offset) {
                rc = written < 0 ? static_cast<int>(written) : -EIO;
                break;
            }
            offset += static_cast<size_t>(written);
        }
    }
    if (rc == 0) {
        rc = fs_sync(&file);
    }
    int close_rc = fs_close(&file);
    if (rc == 0) {
        rc = close_rc;
    }
    if (rc == 0) {
        rc = fs_rename(temporary, path);
    }
    if (rc != 0) {
        (void)fs_unlink(temporary);
    }
    return rc;
}

static inline int commit_snapshot(const char *path, const uint8_t *data, size_t size)
{
    return commit_snapshot_parts(path, nullptr, 0, data, size);
}

} /* namespace storage_detail */

} /* namespace meshbus::arduboy */

#endif /* MESHBUS_ARDUBOY_STORAGE_HPP_ */
