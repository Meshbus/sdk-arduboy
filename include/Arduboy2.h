/* SPDX-License-Identifier: Apache-2.0 AND BSD-3-Clause AND BSD-2-Clause */
/* Original SDK portions: Apache-2.0. drawCircle is adapted from Arduboy2, including
 * its Adafruit-GFX ancestry. See LICENSING.md for the applicable notices.
 */

#ifndef MESHBUS_ARDUBOY2_H_
#define MESHBUS_ARDUBOY2_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <Arduino.h>
#include <Print.h>
#ifdef MESHBUS_ARDUBOY_RUNTIME
#include <meshbus_arduboy/runtime.hpp>
#endif
#include <meshbus_arduboy/font5x7.hpp>
#include <meshbus_arduboy/compat.hpp>

#undef ARDUBOY_LIB_VER
#define ARDUBOY_LIB_VER 50200

static inline char *ltoa(long value, char *str, int base)
{
	static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char tmp[sizeof(unsigned long) * 8U + 1U];
	bool neg = false;
	unsigned long v;
	size_t pos = 0;

	if (base < 2 || base > 36) {
		str[0] = '\0';
		return str;
	}

	if (value < 0 && base == 10) {
		neg = true;
		v = 0UL - static_cast<unsigned long>(value);
	} else {
		v = static_cast<unsigned long>(value);
	}

	do {
		tmp[pos++] = digits[v % static_cast<unsigned long>(base)];
		v /= static_cast<unsigned long>(base);
	} while (v != 0U && pos < sizeof(tmp));

	size_t out = 0;
	if (neg) {
		str[out++] = '-';
	}
	while (pos > 0U) {
		str[out++] = tmp[--pos];
	}
	str[out] = '\0';
	return str;
}

#ifdef MESHBUS_ARDUBOY_RUNTIME
extern "C" bool meshbus_arduboy_audio_enabled();
extern "C" void meshbus_arduboy_audio_begin();
extern "C" void meshbus_arduboy_audio_set_enabled(bool);
extern "C" void meshbus_arduboy_audio_save();
class Arduboy2Audio {
public:
    void begin() { meshbus_arduboy_audio_begin(); }
    static bool enabled() { return meshbus_arduboy_audio_enabled(); }
    static void on() { meshbus_arduboy_audio_set_enabled(true); }
    static void off() { meshbus_arduboy_audio_set_enabled(false); }
    static void saveOnOff() { meshbus_arduboy_audio_save(); }
};
#else
class Arduboy2Audio {
public:
	void begin()
	{
        MESHBUS_ARDUBOY_UNSUPPORTED(legacy_audio_settings);
	}

	static bool enabled()
	{
		return enabled_state_;
	}

	static void on()
	{
		enabled_state_ = true;
	}

	static void off()
	{
		enabled_state_ = false;
	}

	static void saveOnOff()
	{
        MESHBUS_ARDUBOY_UNSUPPORTED(legacy_audio_settings);
	}

private:
	static bool enabled_state_;
};

#endif

class Arduboy2Base {
public:
	Arduboy2Audio audio;
	uint16_t frameCount = 0;

	void begin()
	{
		boot();
	}

	void boot()
	{
		clear();
	}

	void flashlight()
	{
        MESHBUS_ARDUBOY_UNSUPPORTED(flashlight);
	}

	void systemButtons()
	{
        MESHBUS_ARDUBOY_UNSUPPORTED(systemButtons);
	}

	void setFrameRate(uint8_t fps)
	{
		frame_rate_ = fps;
#ifdef MESHBUS_ARDUBOY_RUNTIME
        meshbus_arduboy_set_frame_rate(fps);
#endif
	}

	bool nextFrame()
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        if (frameCount != frame_count_synced_) { meshbus_arduboy_set_frame_count(frameCount); }
        bool ready = meshbus_arduboy_next_frame();
        frameCount = meshbus_arduboy_frame_count();
        frame_count_synced_ = frameCount;
        return ready;
#else
        MESHBUS_ARDUBOY_UNSUPPORTED(legacy_frame_clock);
        ++frameCount;
        return true;
#endif
	}

	bool nextFrameDEV()
	{
		return nextFrame();
	}

	uint8_t cpuLoad() const
    {
#ifdef MESHBUS_ARDUBOY_RUNTIME
        return meshbus_arduboy_cpu_load();
#else
        MESHBUS_ARDUBOY_UNSUPPORTED(legacy_cpu_load);
        return 0U;
#endif
    }

	uint8_t *getBuffer()
	{
		return meshbus_arduboy_framebuffer();
	}

	void clear()
	{
		fillScreen(BLACK);
	}

	void fillScreen(uint8_t color)
	{
		meshbus::arduboy::fill_screen(color);
	}

	void display(bool clear_after = false)
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        meshbus_arduboy_display(clear_after);
#else
		if (clear_after) { clear(); }
#endif
	}

	void pollButtons()
	{
		previous_buttons_ = buttons_;
		buttons_ = meshbus_arduboy_buttons();
	}

	bool pressed(uint8_t buttons) const
	{
		return (meshbus_arduboy_buttons() & buttons) == buttons;
	}

	bool justPressed(uint8_t buttons) const
	{
		return (buttons_ & buttons) != 0U && (previous_buttons_ & buttons) == 0U;
	}

	bool justReleased(uint8_t buttons) const
	{
		return (buttons_ & buttons) == 0U && (previous_buttons_ & buttons) != 0U;
	}

	bool everyXFrames(uint8_t frames) const
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        return frames == 0U ? true : (meshbus_arduboy_frame_count() % frames) == 0U;
#else
        return frames == 0U ? true : (frameCount % frames) == 0U;
#endif
	}

	void drawPixel(int16_t x, int16_t y, uint8_t color = WHITE)
	{
		meshbus::arduboy::draw_pixel(x, y, color);
	}

	void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color)
	{
		meshbus::arduboy::fill_rect(x, y, w, h, color);
	}

	void drawFastHLine(int16_t x, int16_t y, uint8_t w, uint8_t color = WHITE)
	{
		meshbus::arduboy::draw_fast_hline(x, y, w, color);
	}

	void drawFastVLine(int16_t x, int16_t y, uint8_t h, uint8_t color = WHITE)
	{
		meshbus::arduboy::draw_fast_vline(x, y, h, color);
	}

	void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h,
			uint8_t color = WHITE)
	{
		meshbus::arduboy::draw_bitmap(x, y, bitmap, w, h, color);
	}

	void digitalWriteRGB(uint8_t red, uint8_t green, uint8_t blue)
	{
		MESHBUS_ARDUBOY_UNSUPPORTED(RGB);
		(void)red;
		(void)green;
		(void)blue;
	}


 void initRandomSeed() { randomSeed(micros()); }
 // Runtime builds wait on the cancellable application-thread clock.
 void delayShort(uint16_t ms) { delay(ms); }
 void drawSlowXYBitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                       uint8_t w, uint8_t h, uint8_t color = WHITE) {
   meshbus::arduboy::draw_xy_bitmap(x, y, bitmap, w, h, color);
 }
 void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color = WHITE) {
   if (!w || !h) return;
   drawFastHLine(x,y,w,color); drawFastHLine(x,y+h-1,w,color);
   drawFastVLine(x,y,h,color); drawFastVLine(x+w-1,y,h,color);
 }
	void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
	{
        meshbus::arduboy::draw_line(x0, y0, x1, y1, color);
    }

// Circle rasterizer adapted from Arduboy2; see LICENSING.md.
void drawCircle(int16_t x0, int16_t y0, uint8_t r, uint8_t color)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0, y0+r, color);
  drawPixel(x0, y0-r, color);
  drawPixel(x0+r, y0, color);
  drawPixel(x0-r, y0, color);

  while (x<y)
  {
    if (f >= 0)
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }

    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

private:
	uint8_t frame_rate_ = 60U;
	uint8_t buttons_ = 0U;
	uint8_t previous_buttons_ = 0U;
	uint16_t frame_count_synced_ = 0U;
};


// Minimal Arduboy2 text surface required by Bounce (fixed 5x7 font).
// This does not implement the complete Arduino Print/Arduboy2 API.
class Arduboy2 : public Arduboy2Base, public Print {
public:
 void clear() { Arduboy2Base::clear(); cursor_x = cursor_y = 0; }
 void setCursor(int16_t x, int16_t y) { cursor_x=x; cursor_y=y; }
 size_t write(uint8_t c) override {
   if (c == '\r') return 1;
   if (c == '\n' || cursor_x > WIDTH-5) { cursor_x=0; cursor_y+=8; }
   if (c == '\n') return 1;
   for (uint8_t x=0; x<6; ++x) {
     uint8_t bits=x<5 ? meshbus_arduboy_font5x7[c*5+x] : 0;
     for (uint8_t y=0; y<8; ++y) drawPixel(cursor_x+x,cursor_y+y,(bits>>y)&1);
   }
   cursor_x+=6; return 1;
 }
private:
 int16_t cursor_x=0, cursor_y=0;
};

class Sprites {
public:
	void drawOverwrite(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
	{
		meshbus::arduboy::draw_sprite_frame(x, y, bitmap, frame, false, false);
	}

	void drawSelfMasked(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
	{
		meshbus::arduboy::draw_sprite_frame(x, y, bitmap, frame, true, false);
	}

	void drawPlusMask(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame)
	{
		meshbus::arduboy::draw_sprite_frame(x, y, bitmap, frame, false, true);
	}

	void drawExternalMask(int16_t x, int16_t y, const uint8_t *bitmap, const uint8_t *mask,
			      uint8_t frame, uint8_t mask_frame)
	{
		meshbus::arduboy::draw_external_mask(x, y, bitmap, mask, frame, mask_frame);
	}
};

#endif /* MESHBUS_ARDUBOY2_H_ */
