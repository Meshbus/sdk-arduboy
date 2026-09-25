/* SPDX-License-Identifier: Apache-2.0 */
#include <meshbus_arduboy/resources.hpp>
#include <cassert>
#include <algorithm>
#include <vector>

using namespace meshbus::arduboy;
std::vector<uint8_t> file;
int handles, seeks, mode;
int fs_open(fs_file_t *f, const char *, int flags)
{
    assert(flags == FS_O_READ);
    if (mode == 1) { return -ENOENT; }
    ++handles; f->offset = 0; return 0;
}
int fs_close(fs_file_t *) { --handles; return 0; }
int fs_seek(fs_file_t *f, off_t offset, int origin)
{
    assert(origin == FS_SEEK_SET); ++seeks;
    if (mode == 2) { return -EIO; }
    f->offset = offset; return 0;
}
ssize_t fs_read(fs_file_t *f, void *out, size_t size)
{
    if (mode == 3) { return -EIO; }
    if (f->offset >= file.size()) { return 0; }
    size = std::min(size, file.size() - f->offset);
    if (mode == 4) { size = std::min(size, size_t(7)); }
    memcpy(out, file.data() + f->offset, size); f->offset += size; return size;
}
int main()
{
    std::vector<uint8_t> payload(777);
    for (size_t i = 0; i < payload.size(); ++i) { payload[i] = uint8_t(i * 37); }
    ResourceSpec spec{"/extra/apps/fixture.abr", "fixture", 7, 777,
                      save_format::crc32(payload.data(), payload.size())};
    auto reset = [&] {
        file.assign(64, 0); memcpy(file.data(), "MARB", 4);
        save_format::write16(file.data() + 4, 1); save_format::write16(file.data() + 6, 64);
        save_format::write32(file.data() + 8, spec.version);
        save_format::write32(file.data() + 12, spec.length);
        save_format::write32(file.data() + 16, spec.crc32);
        memcpy(file.data() + 24, spec.identity, strlen(spec.identity));
        save_format::write32(file.data() + 60, save_format::crc32(file.data(), 60));
        file.insert(file.end(), payload.begin(), payload.end()); mode = 0;
    };
    reset(); ResourceBundle bundle;
    assert(bundle.open(spec) == 0 && handles == 1 && bundle.length() == 777);
    uint8_t got[300];
    assert(bundle.read(250, got, sizeof(got)) == 0);
    assert(memcmp(got, payload.data() + 250, sizeof(got)) == 0 && seeks == 3);
    assert(bundle.read(520, got, 8) == 0 && seeks == 3);
    assert(bundle.read(777, nullptr, 0) == 0);
    assert(bundle.read(UINT32_MAX, got, 1) == -ERANGE);
    assert(bundle.read(700, got, SIZE_MAX) == -ERANGE);
    assert(bundle.open(spec) == -EBUSY);
    mode = 2; assert(bundle.read(0, got, 1) == -EIO); mode = 0;
    assert(bundle.close() == 0 && handles == 0 && bundle.length() == 0);
    assert(bundle.read(0, got, 1) == -ENODEV);
    for (int failure = 0; failure < 6; ++failure) {
        reset();
        if (failure == 0) { mode = 1; }
        if (failure == 1) { file.pop_back(); }
        if (failure == 2) { file[64] ^= 1; }
        if (failure == 3) { ++spec.version; }
        if (failure == 4) { mode = 3; }
        if (failure == 5) { file.push_back(0); }
        assert(bundle.open(spec) < 0 && handles == 0);
        if (failure == 3) { --spec.version; }
    }
    reset(); mode = 4;
    assert(bundle.open(spec) == 0); assert(bundle.read(250, got, 300) == 0);
    assert(memcmp(got, payload.data() + 250, 300) == 0);
    assert(bundle.close() == 0 && handles == 0);
    reset(); spec.identity = "";
    assert(bundle.open(spec) == -EINVAL && handles == 0);
    spec.identity = "wrong";
    assert(bundle.open(spec) == -EXDEV && handles == 0);
}
