#pragma once
#include <stdint.h>
#include <zephyr/kernel.h>
struct indicator_buzzer_note { uint16_t freq_hz, duration_ms; };
struct indicator_buzzer_melody { const indicator_buzzer_note *notes; uint8_t length; };
#define INDICATOR_SOURCE_SYSTEM 0
namespace indicator_fixture {
inline bool ready, active;
inline int result;
inline uint32_t token;
inline uint64_t deadline;
inline const indicator_buzzer_note *notes;
}
inline bool mbs_indicator_buzzer_is_ready() { return indicator_fixture::ready; }
inline int mbs_indicator_buzzer_play_owned(int, const indicator_buzzer_melody *melody, uint32_t *token)
{
    using namespace indicator_fixture;
    *token = 0;
    if (!ready) { return -ENODEV; }
    if (result) { return result; }
    notes = melody->notes; deadline = kernel_fixture::ticks;
    for (unsigned i=0; i<melody->length; ++i) {
        if (!notes[i].duration_ms) { deadline = UINT64_MAX; break; }
        deadline += (uint64_t(notes[i].duration_ms) * 31250 + 999) / 1000;
    }
    active = true; *token = ++indicator_fixture::token; return 0;
}
inline bool mbs_indicator_buzzer_playing(uint32_t token)
{
    if (kernel_fixture::ticks >= indicator_fixture::deadline) { indicator_fixture::active = false; }
    return token && token == indicator_fixture::token && indicator_fixture::active;
}
inline void mbs_indicator_buzzer_stop_owned(uint32_t token)
{
    if (token && token == indicator_fixture::token) { indicator_fixture::active = false; }
}
inline int mbs_indicator_buzzer_play(int source, const indicator_buzzer_melody *melody)
{
    uint32_t token; return mbs_indicator_buzzer_play_owned(source, melody, &token);
}
inline void mbs_indicator_buzzer_stop() { indicator_fixture::active = false; }

inline int mbs_indicator_buzzer_play_owned_repeat(int source, const indicator_buzzer_melody *melody, uint32_t *token) {
 for(unsigned i=0;i<melody->length;++i) if(!melody->notes[i].duration_ms) return -EINVAL;
 int rc=mbs_indicator_buzzer_play_owned(source,melody,token);
 if(!rc) indicator_fixture::deadline=UINT64_MAX;
 return rc;
}
