#include <cassert>
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
#define MESHBUS_ARDUBOY_RUNTIME 1
#include "../src/runtime.cpp"
#include <Arduboy.h>
static_assert(sizeof(byte)==1);
static const uint16_t finite[] = {0x90,69,0,260,0x80,0,25,0xf0};
static const uint16_t repeat[] = {0x90,72,0,10,0x80,0,10,0xe0};
static const uint16_t poly[] = {0x90,69,0x91,72,0,10,0x80,0x81,0xf0};
static const uint16_t bad[] = {0x92,69,0,100,0xf0};
static const uint16_t short_score[] = {0x90};
static const meshbus::arduboy::LegacyScoreSpanV1 spans[] = {
 {finite,8},{repeat,8},{bad,5},{short_score,1},{poly,9}
};
static void setup_audio() {
 meshbus::arduboy::register_legacy_scores_v1(spans,5);
 meshbus_arduboy_score_play(finite);
 assert(meshbus_arduboy_tone_playing());
 assert(indicator_fixture::notes[0].freq_hz==440 && indicator_fixture::notes[0].duration_ms==260);
 assert(indicator_fixture::notes[1].freq_hz==0 && indicator_fixture::notes[1].duration_ms==25);
 kernel_fixture::ticks+=31250;assert(!meshbus_arduboy_tone_playing());
 meshbus_arduboy_score_play(repeat);kernel_fixture::ticks+=31250;
 assert(meshbus_arduboy_tone_playing());
 ++indicator_fixture::token;meshbus_arduboy_score_stop();assert(indicator_fixture::active);
 meshbus_arduboy_score_play(poly);
#if MESHBUS_ARDUBOY_COMPATIBILITY
 assert(meshbus_arduboy_tone_playing() && indicator_fixture::notes[0].freq_hz==440);
#else
 assert(meshbus_arduboy_audio_status()==-ENOTSUP);
#endif
 meshbus_arduboy_score_play(bad);assert(meshbus_arduboy_audio_status()==-ENOTSUP);
 meshbus_arduboy_score_play(short_score);assert(meshbus_arduboy_audio_status()==-EINVAL);
 uint16_t unregistered[]={0xf0};meshbus_arduboy_score_play(unregistered);
 assert(meshbus_arduboy_audio_status()==-EINVAL && !meshbus_arduboy_tone_playing());
}
static void step() {}
int main() {
 indicator_fixture::ready=true;
 zui_host host{};mbs_desktop_app_args args{&host};
 meshbus::arduboy::SketchConfig config{"legacy_audio",setup_audio,step};
 assert(meshbus::arduboy::run_sketch(&args,config)==0);
}
