/* SPDX-License-Identifier: Apache-2.0 */

#ifndef MESHBUS_ARDUBOY_TONES_H_
#define MESHBUS_ARDUBOY_TONES_H_

#include <stddef.h>

#include <Arduino.h>

#define TONES_END 0x8000
#define TONES_REPEAT 0x8001
#define TONE_HIGH_VOLUME 0x8000

#define NOTE_G1 49
#define NOTE_C2 65
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_C3 131
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_CS3H 277
#define NOTE_C4 262
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_GS5 831
#define NOTE_E6 1319
#define NOTE_CS6 1109
#define NOTE_CS7 2217

extern "C" void meshbus_arduboy_tone_play(uint16_t freq, uint16_t duration_ms,
					bool (*enabled_cb)());
extern "C" void meshbus_arduboy_tones_play_pairs(const uint16_t *pairs,
					       size_t pair_value_count,
					       bool (*enabled_cb)());
extern "C" void meshbus_arduboy_tones_play_score(const uint16_t *score,
					       bool (*enabled_cb)());
extern "C" void meshbus_arduboy_tone_stop();
#ifdef MESHBUS_ARDUBOY_RUNTIME
extern "C" bool meshbus_arduboy_tone_playing();
extern "C" int meshbus_arduboy_audio_status();
extern "C" void meshbus_arduboy_audio_unsupported();
extern "C" void meshbus_arduboy_tones_play_score_bounded(const uint16_t *, size_t, bool (*)());
#endif

class ArduboyTones {
public:
	explicit ArduboyTones(bool (*enabled_cb)() = nullptr)
		: enabled_cb_(enabled_cb)
	{
	}

	void tone(uint16_t freq, uint16_t duration_ms = 0U)
	{
		meshbus_arduboy_tone_play(freq, duration_ms, enabled_cb_);
	}

	void tone(uint16_t freq1, uint16_t dur1, uint16_t freq2, uint16_t dur2)
	{
		const uint16_t pairs[] = {freq1, dur1, freq2, dur2};

		meshbus_arduboy_tones_play_pairs(pairs, sizeof(pairs) / sizeof(pairs[0]),
					      enabled_cb_);
	}

	void tone(uint16_t freq1, uint16_t dur1, uint16_t freq2, uint16_t dur2,
		  uint16_t freq3, uint16_t dur3)
	{
		const uint16_t pairs[] = {freq1, dur1, freq2, dur2, freq3, dur3};

		meshbus_arduboy_tones_play_pairs(pairs, sizeof(pairs) / sizeof(pairs[0]),
					      enabled_cb_);
	}

	void tones(const uint16_t *score)
	{
		meshbus_arduboy_tones_play_score(score, enabled_cb_);
	}

#ifdef MESHBUS_ARDUBOY_RUNTIME
    void tones(const uint16_t *score, size_t values)
    {
        meshbus_arduboy_tones_play_score_bounded(score, values, enabled_cb_);
    }
#endif
	void noTone()
	{
		meshbus_arduboy_tone_stop();
	}

	bool playing() const
	{
#ifdef MESHBUS_ARDUBOY_RUNTIME
        return meshbus_arduboy_tone_playing();
#else
        MESHBUS_ARDUBOY_UNSUPPORTED(legacy_audio_state);
        return false;
#endif
	}

	void volumeMode(uint8_t mode)
	{
        MESHBUS_ARDUBOY_UNSUPPORTED(audio_volume);
#ifdef MESHBUS_ARDUBOY_RUNTIME
        (void)mode; meshbus_arduboy_audio_unsupported();
#else
        (void)mode;
#endif
	}

private:
	bool (*enabled_cb_)();
};

#endif /* MESHBUS_ARDUBOY_TONES_H_ */
