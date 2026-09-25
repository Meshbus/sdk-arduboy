/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_FX_H_
#define MESHBUS_ARDUBOY_FX_H_
#ifndef MESHBUS_ARDUBOY_FX
#error "sdk-arduboy FX requires the explicit FX and RESOURCE build options"
#endif
#include <Arduboy2.h>
#include <meshbus_arduboy/runtime.hpp>
#include <meshbus_arduboy/fx_data.hpp>
#include <zephyr/sys/printk.h>

using uint24_t = uint32_t;
constexpr bool CLEAR_BUFFER = true;
constexpr uint8_t dbmNormal = 0;
constexpr uint8_t dbmMasked = 16;

/** Read-only FX subset verified with Isojourn. No SPI, save or flash writes. */
class FX {
public:
    static void begin(uint16_t page)
    {
        error_ = 0;
        int rc = data_.begin(page);
        if (rc) { fail(rc); }
    }
    // Bus arbitration is unnecessary with the owned software framebuffer.
    static void disableOLED() {}
    static void enableOLED() {}
    static void display(bool clear = false) { meshbus_arduboy_display(clear); }
    static int lastError() { return error_; }
    static uint32_t readCount() { return data_.reads; }
    static uint32_t decodeCount() { return data_.decodes; }

    static void readDataArray(uint24_t address, uint8_t index, uint8_t offset,
                              uint8_t elementSize, uint8_t *buffer, size_t length)
    {
        if (error_) { return; }
        uint32_t displacement = uint32_t(index) * elementSize + offset;
        if (address > 0xffffffU || displacement > 0xffffffU - address) { fail(-ERANGE); return; }
        int rc = data_.read(address + displacement, buffer, length);
        if (rc) { fail(rc); }
    }
    static void drawBitmap(int16_t x, int16_t y, uint24_t address, uint8_t frame, uint8_t mode)
    {
        if (error_) { return; }
        if (mode != dbmNormal && mode != dbmMasked) { fail(-ENOTSUP); return; }
        uint8_t header[4];
        int rc = data_.read(address, header, sizeof(header));
        if (rc) { fail(rc); return; }
        // Upstream FX dimensions are big-endian; pixel bytes are vertical LSB.
        uint16_t width = (uint16_t(header[0]) << 8) | header[1];
        uint16_t height = (uint16_t(header[2]) << 8) | header[3];
        if (!width || !height || width > 128 || height > 128) { fail(-ENOTSUP); return; }
        uint32_t stride = mode == dbmMasked ? 2U : 1U;
        uint32_t frame_size = width * ((height + 7U) / 8U) * stride;
        if ((uint32_t(frame) + 1U) * frame_size > data_.length() - address - 4U) {
            fail(-ERANGE); return;
        }
        if (x >= 128 || y >= 64 || int32_t(x) + width <= 0 || int32_t(y) + height <= 0) { return; }
        int first = x < 0 ? -x : 0;
        int last = width;
        if (last > 128 - x) { last = 128 - x; }
        auto *buffer = meshbus_arduboy_framebuffer();
        if (!buffer) { fail(-ENODEV); return; }
        uint8_t row[256];
        uint32_t base = address + 4U + uint32_t(frame) * frame_size;
        for (unsigned page = 0; page < (height + 7U) / 8U; ++page) {
            int top = y + int(page) * 8;
            if (top >= 64 || top + 8 <= 0) { continue; }
            rc = data_.read(base + (page * width + first) * stride, row, (last - first) * stride);
            if (rc) { fail(rc); return; }
            for (int col = first; col < last; ++col) {
                unsigned at = (col - first) * stride;
                for (unsigned bit = 0; bit < 8 && page * 8 + bit < height; ++bit) {
                    if (stride == 1 || (row[at + 1] & (1U << bit))) {
                        meshbus::arduboy::draw_pixel_to(buffer, x + col, top + static_cast<int>(bit),
                                                      (row[at] >> bit) & 1U);
                    }
                }
            }
        }
    }
private:
    static void fail(int rc)
    {
        if (!error_) { printk("[arduboy] FX request failed: %d\n", rc); }
        error_ = rc;
        meshbus::arduboy::request_exit();
    }
    inline static meshbus::arduboy::FxData data_;
    inline static int error_ = -ENODEV;
};
#endif
