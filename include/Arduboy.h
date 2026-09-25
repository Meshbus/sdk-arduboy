/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_H_
#define MESHBUS_ARDUBOY_H_

#include <meshbus_arduboy/capabilities.hpp>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <meshbus_arduboy/compat.hpp>
#include <Arduino.h>
#ifdef MESHBUS_ARDUBOY_RUNTIME
#include <meshbus_arduboy/runtime.hpp>
extern "C" bool meshbus_arduboy_tone_playing();
extern "C" void meshbus_arduboy_audio_set_enabled(bool);
#endif

#if !MESHBUS_ARDUBOY_COMPATIBILITY && !defined(MESHBUS_ARDUBOY_RUNTIME)
#error "Legacy Arduboy requires explicit COMPATIBILITY; runtime migration is separate"
#endif

#ifndef ARDUBOY_LIB_VER
#define ARDUBOY_LIB_VER 10101
#endif

#define EEPROM_STORAGE_SPACE_START 0
#ifdef MESHBUS_ARDUBOY_RUNTIME
#define EEPROM_AUDIO_ON_OFF 2
#else
#define EEPROM_AUDIO_ON_OFF 0
#endif
#define OLED_ALL_PIXELS_ON 0xa5
#define PIN_SPEAKER_1 0
#define PIN_SPEAKER_2 1
#define OUTPUT 1

// Arduino.h owns byte = uint8_t in both modes.
using buffer_t = uint8_t;

#define rand meshbus_arduboy_rand

static inline uint8_t *pgm_read_word(const uint8_t *const *addr)
{
	/* Legacy Arduboy games used word for native pointer tables. */
	return const_cast<uint8_t *>(pgm_read_ptr(addr));
}

/* Hollow's original menu tables use AVR word reads for string pointers. */
static inline const char *pgm_read_word(const char *const *addr)
{
    return pgm_read_ptr(addr);
}

static inline void eeprom_busy_wait()
{
}

static inline uint8_t eeprom_read_byte(const uint8_t *addr)
{
	return meshbus_arduboy_eeprom_read(reinterpret_cast<uintptr_t>(addr));
}

static inline uint16_t eeprom_read_word(const uint16_t *addr)
{
	size_t offset = reinterpret_cast<uintptr_t>(addr);

	return static_cast<uint16_t>(meshbus_arduboy_eeprom_read(offset)) |
	       (static_cast<uint16_t>(meshbus_arduboy_eeprom_read(offset + 1U)) << 8);
}

static inline uint32_t eeprom_read_dword(const uint32_t *addr)
{
	size_t offset = reinterpret_cast<uintptr_t>(addr);

	return static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset)) |
	       (static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset + 1U)) << 8) |
	       (static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset + 2U)) << 16) |
	       (static_cast<uint32_t>(meshbus_arduboy_eeprom_read(offset + 3U)) << 24);
}

static inline void eeprom_read_block(void *dst, const void *addr, size_t len)
{
	size_t offset = reinterpret_cast<uintptr_t>(addr);

	meshbus_arduboy_eeprom_read_block(dst, offset, len);
}

static inline void eeprom_write_byte(uint8_t *addr, uint8_t value)
{
	meshbus_arduboy_eeprom_write(reinterpret_cast<uintptr_t>(addr), value);
}

static inline void eeprom_write_word(uint16_t *addr, uint16_t value)
{
	size_t offset = reinterpret_cast<uintptr_t>(addr);

	meshbus_arduboy_eeprom_write(offset, static_cast<uint8_t>(value));
	meshbus_arduboy_eeprom_write(offset + 1U, static_cast<uint8_t>(value >> 8));
}

static inline void eeprom_write_dword(uint32_t *addr, uint32_t value)
{
	size_t offset = reinterpret_cast<uintptr_t>(addr);

	for (size_t i = 0U; i < 4U; i++) {
		meshbus_arduboy_eeprom_write(offset + i,
					     static_cast<uint8_t>(value >> (i * 8U)));
	}
}

static inline void eeprom_write_block(const void *src, void *addr, size_t len)
{
	size_t offset = reinterpret_cast<uintptr_t>(addr);

	meshbus_arduboy_eeprom_write_block(src, offset, len);
}

class MeshbusArduboyEEPROM {
public:
	uint8_t read(int addr) const
	{
		return addr < 0 ? 0xffU : meshbus_arduboy_eeprom_read(static_cast<size_t>(addr));
	}
};

static MeshbusArduboyEEPROM EEPROM;

class ArduboyAudio {
public:
	static void begin()
	{
	}

	static bool enabled()
	{
		return audio_enabled;
	}

	static void on()
	{
		audio_enabled = true;
#ifdef MESHBUS_ARDUBOY_RUNTIME
        meshbus_arduboy_audio_set_enabled(true);
#endif
	}

	static void off()
	{
		audio_enabled = false;
#ifdef MESHBUS_ARDUBOY_RUNTIME
        meshbus_arduboy_audio_set_enabled(false);
#endif
	}

	static void saveOnOff()
	{
		meshbus_arduboy_eeprom_write(EEPROM_AUDIO_ON_OFF, audio_enabled ? 1U : 0U);
	}

protected:
	static bool audio_enabled;
};

class ArduboyTunes {
public:
	void initChannel(uint8_t pin)
	{
		(void)pin;
	}

	bool playing() const
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        return meshbus_arduboy_tone_playing();
#else
        return false;
#endif
    }

    void playScore(const uint16_t *score)
	{
		meshbus_arduboy_score_play(score);
	}

	void stopScore()
	{
		meshbus_arduboy_score_stop();
	}
};

class Arduboy {
public:
	ArduboyAudio audio;
	ArduboyTunes tunes;

	void boot()
	{
	}

	void blank()
	{
		clear();
	}

	void flashlight()
	{
	}

	void systemButtons()
	{
	}

	void setFrameRate(uint8_t fps)
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        meshbus_arduboy_set_frame_rate(fps);
#else
        frame_rate_ = fps;
#endif
	}

	bool nextFrame()
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        return meshbus_arduboy_next_frame();
#else
        frame_count_++;
        return true;
#endif
	}

	void clear()
	{
		meshbus::arduboy::fill_screen(BLACK);
	}

	void display()
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        meshbus_arduboy_display(false);
#endif
	}

	uint8_t *getBuffer()
	{
		return meshbus_arduboy_framebuffer();
	}

	uint8_t buttonsState() const
	{
		return meshbus_arduboy_buttons();
	}

	bool pressed(uint8_t buttons) const
	{
		return (buttonsState() & buttons) == buttons;
	}

	void setCursor(int16_t x, int16_t y)
	{
		cursor_x = x;
		cursor_y = y;
	}

	size_t print(const char *text)
	{
		size_t count = 0U;

		if (text == nullptr) {
			return 0U;
		}
		while (*text != '\0') {
			count += write(static_cast<uint8_t>(*text++));
		}
		return count;
	}

	size_t print(const __FlashStringHelper *text)
	{
		return print(reinterpret_cast<const char *>(text));
	}

	size_t print(char c)
	{
		return write(static_cast<uint8_t>(c));
	}

	size_t print(int value)
	{
		return print(static_cast<long>(value));
	}

	size_t print(unsigned int value)
	{
		return print(static_cast<unsigned long>(value));
	}

	size_t print(long value)
	{
		char buffer[16];

		format_signed(buffer, sizeof(buffer), value);
		return print(buffer);
	}

	size_t print(unsigned long value)
	{
		char buffer[16];

		format_unsigned(buffer, sizeof(buffer), value);
		return print(buffer);
	}

	virtual size_t write(uint8_t c)
	{
		(void)c;
		return 1U;
	}

	void drawPixel(int16_t x, int16_t y, uint8_t color)
	{
		meshbus::arduboy::draw_pixel(x, y, color);
	}

	void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
	{
        meshbus::arduboy::draw_line(x0, y0, x1, y1, color);
    }

	void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h,
			uint8_t color)
	{
		meshbus::arduboy::draw_bitmap(x, y, bitmap, w, h, color);
	}

	void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
	{
		meshbus::arduboy::fill_rect(x, y, w, h, color);
	}

	void drawFastHLine(int16_t x, int16_t y, uint8_t w, uint8_t color)
	{
		meshbus::arduboy::draw_fast_hline(x, y, w, color);
	}

	void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color)
	{
		meshbus::arduboy::draw_fast_vline(x, y, h, color);
	}

	void setRGBled(uint8_t red, uint8_t green, uint8_t blue)
	{
		(void)red;
		(void)green;
		(void)blue;
	}

	void sendLCDCommand(uint8_t cmd)
	{
		(void)cmd;
	}

	void idle()
	{
	}

protected:
	int16_t cursor_x = 0;
	int16_t cursor_y = 0;
	uint8_t textsize = 1;
	bool wrap = true;

private:
	uint8_t frame_rate_ = 60U;
	uint32_t frame_count_ = 0U;

	static void format_unsigned(char *buffer, size_t len, unsigned long value)
	{
		char tmp[16];
		size_t pos = 0U;
		size_t out = 0U;

		if (len == 0U) {
			return;
		}
		do {
			tmp[pos++] = static_cast<char>('0' + (value % 10UL));
			value /= 10UL;
		} while (value != 0UL && pos < sizeof(tmp));

		while (pos > 0U && out + 1U < len) {
			buffer[out++] = tmp[--pos];
		}
		buffer[out] = '\0';
	}

	static void format_signed(char *buffer, size_t len, long value)
	{
		if (len == 0U) {
			return;
		}
		if (value < 0) {
			buffer[0] = '-';
			format_unsigned(&buffer[1], len - 1U, static_cast<unsigned long>(-value));
			return;
		}
		format_unsigned(buffer, len, static_cast<unsigned long>(value));
	}
};

static inline void power_timer0_disable()
{
}

static inline void power_timer3_enable()
{
#ifdef MESHBUS_ARDUBOY_RUNTIME
    meshbus_arduboy_audio_set_enabled(true);
#endif
}

static inline void power_timer3_disable()
{
#ifdef MESHBUS_ARDUBOY_RUNTIME
    meshbus_arduboy_audio_set_enabled(false);
#endif
}

#endif /* MESHBUS_ARDUBOY_H_ */
