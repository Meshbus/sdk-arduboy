/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_COMPAT_HPP_
#define MESHBUS_ARDUBOY_COMPAT_HPP_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zui/zui.h>

#ifndef WIDTH
#define WIDTH 128
#endif

#ifndef HEIGHT
#define HEIGHT 64
#endif

#ifndef BLACK
#define BLACK 0
#endif

#ifndef WHITE
#define WHITE 1
#endif

#ifndef UP_BUTTON
#define UP_BUTTON (1U << 0)
#endif

#ifndef DOWN_BUTTON
#define DOWN_BUTTON (1U << 1)
#endif

#ifndef LEFT_BUTTON
#define LEFT_BUTTON (1U << 2)
#endif

#ifndef RIGHT_BUTTON
#define RIGHT_BUTTON (1U << 3)
#endif

#ifndef A_BUTTON
#define A_BUTTON (1U << 4)
#endif

#ifndef B_BUTTON
#define B_BUTTON (1U << 5)
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

class __FlashStringHelper;

#ifndef F
#define F(str) (reinterpret_cast<const __FlashStringHelper *>(str))
#endif

#ifndef strcpy_P
#define strcpy_P(dst, src) strcpy((dst), (src))
#endif

#ifndef memcpy_P
#define memcpy_P(dst, src, len) memcpy((dst), (src), (len))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#include <meshbus_arduboy/pgmspace.hpp>

extern "C" uint8_t *meshbus_arduboy_framebuffer();
extern "C" uint8_t meshbus_arduboy_buttons();
extern "C" uint32_t meshbus_arduboy_millis();
extern "C" uint32_t meshbus_arduboy_micros();
extern "C" void meshbus_arduboy_random_seed(uint32_t seed);
extern "C" long meshbus_arduboy_random(long max);
extern "C" long meshbus_arduboy_random_range(long min, long max);
extern "C" int meshbus_arduboy_rand();
extern "C" size_t meshbus_arduboy_eeprom_length();
extern "C" uint8_t meshbus_arduboy_eeprom_read(size_t address);
extern "C" void meshbus_arduboy_eeprom_write(size_t address, uint8_t value);
extern "C" void meshbus_arduboy_eeprom_update(size_t address, uint8_t value);
extern "C" void meshbus_arduboy_eeprom_read_block(void *dst, size_t address, size_t len);
extern "C" void meshbus_arduboy_eeprom_write_block(const void *src, size_t address,
						   size_t len);
extern "C" void meshbus_arduboy_eeprom_update_block(const void *src, size_t address,
						    size_t len);
extern "C" void meshbus_arduboy_score_play(const uint16_t *score);
extern "C" void meshbus_arduboy_score_stop();

namespace meshbus::arduboy {

static inline uint8_t buttons_from_zui(uint32_t down, bool cancel_as_b = false)
{
	uint8_t buttons = 0U;

	if ((down & ZUI_ACTION_PRIMARY) != 0U) {
		buttons |= A_BUTTON;
	}
	if ((down & ZUI_ACTION_SECONDARY) != 0U ||
	    (cancel_as_b && (down & ZUI_ACTION_CANCEL) != 0U)) {
		buttons |= B_BUTTON;
	}
	if ((down & ZUI_ACTION_UP) != 0U) {
		buttons |= UP_BUTTON;
	}
	if ((down & ZUI_ACTION_DOWN) != 0U) {
		buttons |= DOWN_BUTTON;
	}
	if ((down & ZUI_ACTION_LEFT) != 0U) {
		buttons |= LEFT_BUTTON;
	}
	if ((down & ZUI_ACTION_RIGHT) != 0U) {
		buttons |= RIGHT_BUTTON;
	}

	return buttons;
}

static inline size_t page_index(int16_t x, int16_t y)
{
	return (static_cast<size_t>(y) / 8U) * WIDTH + static_cast<size_t>(x);
}

static inline bool in_bounds(int16_t x, int16_t y)
{
	return x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT;
}

static inline bool clip_rect(int16_t *x, int16_t *y, uint8_t *w, uint8_t *h)
{
	int16_t x0;
	int16_t y0;
	int16_t x1;
	int16_t y1;

	if (x == nullptr || y == nullptr || w == nullptr || h == nullptr || *w == 0U ||
	    *h == 0U) {
		return false;
	}

	x0 = *x;
	y0 = *y;
	x1 = static_cast<int16_t>(x0 + *w);
	y1 = static_cast<int16_t>(y0 + *h);
	if (x1 <= 0 || y1 <= 0 || x0 >= WIDTH || y0 >= HEIGHT) {
		return false;
	}

	if (x0 < 0) {
		x0 = 0;
	}
	if (y0 < 0) {
		y0 = 0;
	}
	if (x1 > WIDTH) {
		x1 = WIDTH;
	}
	if (y1 > HEIGHT) {
		y1 = HEIGHT;
	}

	*x = x0;
	*y = y0;
	*w = static_cast<uint8_t>(x1 - x0);
	*h = static_cast<uint8_t>(y1 - y0);
	return *w != 0U && *h != 0U;
}

static inline void draw_pixel_to(uint8_t *buffer, int16_t x, int16_t y, uint8_t color)
{
	if (!in_bounds(x, y)) {
		return;
	}

	size_t index = page_index(x, y);
	uint8_t mask = static_cast<uint8_t>(1U << (y & 0x7));

	if (color == BLACK) {
		buffer[index] &= static_cast<uint8_t>(~mask);
	} else {
		buffer[index] |= mask;
	}
}

static inline void draw_pixel(int16_t x, int16_t y, uint8_t color)
{
	uint8_t *buffer = meshbus_arduboy_framebuffer();

	if (buffer == nullptr) {
		return;
	}

	draw_pixel_to(buffer, x, y, color);
}

static inline void apply_masked_byte(uint8_t *buffer, int16_t x, int16_t y, uint8_t data,
				     uint8_t mask)
{
	if (buffer == nullptr || mask == 0U || x < 0 || x >= WIDTH) {
		return;
	}

	if (y < 0) {
		uint8_t shift = static_cast<uint8_t>(-y);

		if (shift >= 8U) {
			return;
		}
		data = static_cast<uint8_t>(data >> shift);
		mask = static_cast<uint8_t>(mask >> shift);
		y = 0;
	}
	if (y >= HEIGHT || mask == 0U) {
		return;
	}

	uint8_t page = static_cast<uint8_t>(y >> 3);
	uint8_t offset = static_cast<uint8_t>(y & 0x7);
	size_t index = static_cast<size_t>(page) * WIDTH + static_cast<size_t>(x);
	uint8_t low_mask = static_cast<uint8_t>(mask << offset);
	uint8_t low_data = static_cast<uint8_t>(data << offset);

	buffer[index] = static_cast<uint8_t>((buffer[index] & ~low_mask) | (low_data & low_mask));

	if (offset == 0U || page + 1U >= HEIGHT / 8U) {
		return;
	}

	index += WIDTH;
	uint8_t high_mask = static_cast<uint8_t>(mask >> (8U - offset));
	uint8_t high_data = static_cast<uint8_t>(data >> (8U - offset));

	buffer[index] = static_cast<uint8_t>((buffer[index] & ~high_mask) |
					     (high_data & high_mask));
}

static inline void apply_or_byte(uint8_t *buffer, int16_t x, int16_t y, uint8_t data)
{
	if (buffer == nullptr || data == 0U || x < 0 || x >= WIDTH) {
		return;
	}

	if (y < 0) {
		uint8_t shift = static_cast<uint8_t>(-y);

		if (shift >= 8U) {
			return;
		}
		data = static_cast<uint8_t>(data >> shift);
		y = 0;
	}
	if (y >= HEIGHT || data == 0U) {
		return;
	}

	uint8_t page = static_cast<uint8_t>(y >> 3);
	uint8_t offset = static_cast<uint8_t>(y & 0x7);
	size_t index = static_cast<size_t>(page) * WIDTH + static_cast<size_t>(x);

	buffer[index] |= static_cast<uint8_t>(data << offset);

	if (offset != 0U && page + 1U < HEIGHT / 8U) {
		buffer[index + WIDTH] |= static_cast<uint8_t>(data >> (8U - offset));
	}
}

static inline uint8_t mask_for_height(uint8_t height)
{
	return height >= 8U ? 0xffU : static_cast<uint8_t>((1U << height) - 1U);
}

static inline uint8_t valid_sprite_mask(uint8_t height, uint8_t page)
{
	uint8_t remaining = static_cast<uint8_t>(height - page * 8U);

	return mask_for_height(remaining);
}

static inline void fill_screen(uint8_t color)
{
	uint8_t *buffer = meshbus_arduboy_framebuffer();

	if (buffer != nullptr) {
		memset(buffer, color == BLACK ? 0x00 : 0xff, WIDTH * HEIGHT / 8U);
	}
}

static inline void fill_rect_to(uint8_t *buffer, int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
{

	if (buffer == nullptr || !clip_rect(&x, &y, &w, &h)) {
		return;
	}

	uint8_t remaining = h;
	int16_t draw_y = y;

	while (remaining > 0U) {
		uint8_t row_offset = static_cast<uint8_t>(draw_y & 0x7);
		uint8_t max_chunk = static_cast<uint8_t>(8U - row_offset);
		uint8_t chunk = remaining < max_chunk ? remaining : max_chunk;
		uint8_t mask = static_cast<uint8_t>(mask_for_height(chunk) << row_offset);
		size_t index = static_cast<size_t>(draw_y >> 3) * WIDTH + static_cast<size_t>(x);

		for (uint8_t px = 0U; px < w; px++) {
			if (color == BLACK) {
				buffer[index + px] &= static_cast<uint8_t>(~mask);
			} else {
				buffer[index + px] |= mask;
			}
		}

		draw_y = static_cast<int16_t>(draw_y + chunk);
		remaining = static_cast<uint8_t>(remaining - chunk);
	}
}

static inline void fill_rect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
{
	fill_rect_to(meshbus_arduboy_framebuffer(), x, y, w, h, color);
}

static inline void draw_fast_hline(int16_t x, int16_t y, uint8_t w, uint8_t color)
{
	fill_rect(x, y, w, 1U, color);
}

static inline void draw_fast_vline(int16_t x, int16_t y, uint8_t h, uint8_t color)
{
	fill_rect(x, y, 1U, h, color);
}

static inline bool bitmap_bit(const uint8_t *data, uint8_t width, uint8_t x, uint8_t y)
{
	size_t index = static_cast<size_t>(x) + (static_cast<size_t>(y) / 8U) * width;
	uint8_t mask = static_cast<uint8_t>(1U << (y & 0x7U));

	return (pgm_read_byte(data + index) & mask) != 0U;
}

static inline void draw_bitmap_to(uint8_t *buffer, int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w,
			       uint8_t h, uint8_t color)
{
	if (bitmap == nullptr) {
		return;
	}


	if (buffer == nullptr) {
		return;
	}

	for (uint8_t page = 0U; page < (h + 7U) / 8U; page++) {
		uint8_t valid_mask = valid_sprite_mask(h, page);

		for (uint8_t px = 0U; px < w; px++) {
			size_t byte_index = static_cast<size_t>(page) * w + px;
			uint8_t source = static_cast<uint8_t>(pgm_read_byte(bitmap + byte_index) &
							     valid_mask);
			uint8_t data = color == BLACK ? 0U : source;

			apply_masked_byte(buffer, x + px, y + page * 8U, data, source);
		}
	}
}

static inline void draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w,
			       uint8_t h, uint8_t color)
{
	draw_bitmap_to(meshbus_arduboy_framebuffer(), x, y, bitmap, w, h, color);
}

static inline void draw_xy_bitmap_to(uint8_t *buffer, int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w,
				  uint8_t h, uint8_t color)
{
	if (bitmap == nullptr) {
		return;
	}


	if (buffer == nullptr) {
		return;
	}

	uint8_t byte_width = static_cast<uint8_t>((w + 7U) / 8U);

	for (uint8_t py = 0U; py < h; py++) {
		int16_t dst_y = static_cast<int16_t>(y + py);

		if (dst_y < 0 || dst_y >= HEIGHT) {
			continue;
		}

		for (uint8_t px = 0U; px < w; px++) {
			int16_t dst_x = static_cast<int16_t>(x + px);
			uint8_t byte = pgm_read_byte(bitmap + static_cast<size_t>(py) * byte_width +
						     px / 8U);

			if (dst_x >= 0 && dst_x < WIDTH && (byte & (0x80U >> (px & 0x7U))) != 0U) {
				draw_pixel_to(buffer, dst_x, dst_y, color);
			}
		}
	}
}

static inline void draw_xy_bitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w,
				  uint8_t h, uint8_t color)
{
	draw_xy_bitmap_to(meshbus_arduboy_framebuffer(), x, y, bitmap, w, h, color);
}

static inline void draw_sprite_frame(int16_t x, int16_t y, const uint8_t *bitmap,
				     uint8_t frame, bool self_masked, bool plus_mask)
{
	if (bitmap == nullptr) {
		return;
	}

	uint8_t width = pgm_read_byte(bitmap);
	uint8_t height = pgm_read_byte(bitmap + 1);
	size_t bytes_per_frame = static_cast<size_t>(width) * ((height + 7U) / 8U);
	const uint8_t *frame_data = bitmap + 2U;

	if (plus_mask) {
		frame_data += static_cast<size_t>(frame) * bytes_per_frame * 2U;
	} else {
		frame_data += static_cast<size_t>(frame) * bytes_per_frame;
	}

	uint8_t *buffer = meshbus_arduboy_framebuffer();

	if (buffer == nullptr) {
		return;
	}

	for (uint8_t page = 0U; page < (height + 7U) / 8U; page++) {
		uint8_t valid_mask = valid_sprite_mask(height, page);

		for (uint8_t px = 0U; px < width; px++) {
			size_t byte_index = static_cast<size_t>(page) * width + px;
			uint8_t data;
			uint8_t mask;

			if (plus_mask) {
				size_t pair_index = byte_index * 2U;

				data = static_cast<uint8_t>(pgm_read_byte(frame_data + pair_index) &
							   valid_mask);
				mask = static_cast<uint8_t>(pgm_read_byte(frame_data + pair_index + 1U) &
							   valid_mask);
				apply_masked_byte(buffer, x + px, y + page * 8U, data, mask);
			} else {
				data = static_cast<uint8_t>(pgm_read_byte(frame_data + byte_index) &
							   valid_mask);
				if (self_masked) {
					apply_or_byte(buffer, x + px, y + page * 8U, data);
				} else {
					apply_masked_byte(buffer, x + px, y + page * 8U, data,
							  valid_mask);
				}
			}
		}
	}
}

static inline void draw_external_mask(int16_t x, int16_t y, const uint8_t *bitmap,
				      const uint8_t *mask, uint8_t frame, uint8_t mask_frame)
{
	if (bitmap == nullptr || mask == nullptr) {
		return;
	}

	uint8_t width = pgm_read_byte(bitmap);
	uint8_t height = pgm_read_byte(bitmap + 1);
	size_t bytes_per_frame = static_cast<size_t>(width) * ((height + 7U) / 8U);
	const uint8_t *bitmap_frame = bitmap + 2U + static_cast<size_t>(frame) * bytes_per_frame;
	const uint8_t *mask_frame_data = mask + static_cast<size_t>(mask_frame) * bytes_per_frame;

	uint8_t *buffer = meshbus_arduboy_framebuffer();

	if (buffer == nullptr) {
		return;
	}

	for (uint8_t page = 0U; page < (height + 7U) / 8U; page++) {
		uint8_t valid_mask = valid_sprite_mask(height, page);

		for (uint8_t px = 0U; px < width; px++) {
			size_t byte_index = static_cast<size_t>(page) * width + px;
			uint8_t data = static_cast<uint8_t>(pgm_read_byte(bitmap_frame + byte_index) &
							   valid_mask);
			uint8_t draw = static_cast<uint8_t>(pgm_read_byte(mask_frame_data + byte_index) &
							   valid_mask);

			apply_masked_byte(buffer, x + px, y + page * 8U, data, draw);
		}
	}
}

/* Wide intermediates preserve clipping for lines crossing the viewport. */
static inline void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                             uint8_t color)
{
    int32_t x = x0, y = y0;
    const int32_t dx = abs(static_cast<int32_t>(x1) - x0);
    const int32_t dy = -abs(static_cast<int32_t>(y1) - y0);
    const int32_t sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;
    for (;;) {
        draw_pixel(static_cast<int16_t>(x), static_cast<int16_t>(y), color);
        if (x == x1 && y == y1) { break; }
        int32_t twice = 2 * error;
        if (twice >= dy) { error += dy; x += sx; }
        if (twice <= dx) { error += dx; y += sy; }
    }
}

} /* namespace meshbus::arduboy */

#endif /* MESHBUS_ARDUBOY_COMPAT_HPP_ */
