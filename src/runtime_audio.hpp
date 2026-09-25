/* SPDX-License-Identifier: Apache-2.0 */
namespace {
constexpr size_t max_tone_notes = 32;
indicator_buzzer_note owned_notes[max_tone_notes];
uint32_t tone_token, tone_frame;
int audio_status;
bool audio_enabled = true, audio_silent;
void audio_result(int status)
{
    if (status != 0 && status != -EACCES && status != audio_status) {
        printk("[arduboy] audio request failed: %d\n", status);
    }
    audio_status = status;
}
}
extern "C" int meshbus_arduboy_audio_status() { return audio_status; }
extern "C" bool meshbus_arduboy_audio_enabled() { return audio_enabled; }
extern "C" void meshbus_arduboy_tone_stop()
{
    if (tone_token) { mbs_indicator_buzzer_stop_owned(tone_token); tone_token = 0; }
}
extern "C" bool meshbus_arduboy_tone_playing()
{
    return tone_token && mbs_indicator_buzzer_playing(tone_token);
}
extern "C" void meshbus_arduboy_audio_set_enabled(bool enabled)
{
    audio_enabled = enabled;
    if (!enabled) { meshbus_arduboy_tone_stop(); }
}
extern "C" void meshbus_arduboy_audio_begin()
{
    if (!meshbus_arduboy_runtime_ready()) { return; }
    meshbus_arduboy_audio_set_enabled(meshbus_arduboy_eeprom_read(meshbus::arduboy::eeprom_audio_setting) != 0);
}
extern "C" void meshbus_arduboy_audio_save()
{
    if (meshbus_arduboy_runtime_ready()) {
        meshbus_arduboy_eeprom_update(meshbus::arduboy::eeprom_audio_setting, audio_enabled ? 1 : 0);
    }
}
extern "C" void meshbus_arduboy_audio_unsupported() { audio_result(-ENOTSUP); }
static bool audio_can_play(bool (*enabled_cb)())
{
    meshbus_arduboy_tone_stop(); // Synchronizes prior borrowed-note reads before rewriting.
    if (!meshbus_arduboy_runtime_ready()) { audio_result(-ENXIO); return false; }
    if (audio_silent || !audio_enabled || (enabled_cb && !enabled_cb())) {
        audio_result(-EACCES); return false;
    }
    if (!mbs_indicator_buzzer_is_ready()) { audio_result(-ENODEV); return false; }
    return true;
}
static void audio_submit(size_t count)
{
    if (!count) { audio_result(-EINVAL); return; }
    for (size_t i=0; i+1<count; ++i) {
        if (!owned_notes[i].duration_ms) { audio_result(-ENOTSUP); return; }
    }
    const indicator_buzzer_melody melody{owned_notes, static_cast<uint8_t>(count)};
    audio_result(mbs_indicator_buzzer_play_owned(INDICATOR_SOURCE_SYSTEM, &melody, &tone_token));
}
extern "C" void meshbus_arduboy_tones_play_pairs(const uint16_t *pairs, size_t values,
                                                 bool (*enabled_cb)())
{
    if (!audio_can_play(enabled_cb)) { return; }
    if (!pairs || values < 2 || values % 2) { audio_result(-EINVAL); return; }
    if (values / 2 > max_tone_notes) { audio_result(-E2BIG); return; }
    for (size_t i=0; i<values/2; ++i) {
        if (pairs[i*2] == TONES_REPEAT || pairs[i*2] == TONES_END) {
            audio_result(-ENOTSUP); return;
        }
        owned_notes[i] = {static_cast<uint16_t>(pairs[i*2] & ~TONE_HIGH_VOLUME), pairs[i*2+1]};
    }
    audio_submit(values / 2);
}
extern "C" void meshbus_arduboy_tones_play_score_bounded(const uint16_t *score, size_t values,
                                                         bool (*enabled_cb)())
{
    if (!audio_can_play(enabled_cb)) { return; }
    if (!score) { audio_result(-EINVAL); return; }
    size_t count = 0;
    for (size_t i=0; i<values;) {
        uint16_t freq = pgm_read_word(score + i++);
        if (freq == TONES_END) { audio_submit(count); return; }
        if (freq == TONES_REPEAT) { audio_result(-ENOTSUP); return; }
        if (count == max_tone_notes) { audio_result(-E2BIG); return; }
        if (i == values) { audio_result(-EINVAL); return; }
        owned_notes[count++] = {static_cast<uint16_t>(freq & ~TONE_HIGH_VOLUME), pgm_read_word(score + i++)};
    }
    audio_result(-EINVAL); // No terminator in the declared span: never play a prefix.
}
extern "C" void meshbus_arduboy_tones_play_score(const uint16_t *score, bool (*enabled_cb)())
{
    // Legacy pointer API requires a valid terminated score (at most 32 pairs).
    meshbus_arduboy_tones_play_score_bounded(score, max_tone_notes * 2 + 1, enabled_cb);
}
extern "C" void meshbus_arduboy_tone_play(uint16_t freq, uint16_t duration, bool (*enabled_cb)())
{
    const uint16_t pair[] = {freq, duration};
    meshbus_arduboy_tones_play_pairs(pair, 2, enabled_cb);
}

namespace {
const meshbus::arduboy::LegacyScoreSpanV1 *legacy_scores;
size_t legacy_score_count;
}
void meshbus::arduboy::register_legacy_scores_v1(const LegacyScoreSpanV1 *scores, size_t count)
{
    legacy_scores = scores; legacy_score_count = count;
}
extern "C" void meshbus_arduboy_score_play(const uint16_t *score)
{
    if (!audio_can_play(nullptr)) { return; }
    size_t values = 0;
    for (size_t i=0; i<legacy_score_count; ++i) {
        if (legacy_scores[i].words == score) { values = legacy_scores[i].count; break; }
    }
    if (!values) { audio_result(-EINVAL); return; }
    static constexpr uint16_t frequencies[] = {8,9,9,10,10,11,12,12,13,14,15,15,16,17,18,19,21,22,23,24,26,28,29,31,33,35,37,39,41,44,46,49,52,55,58,62,65,69,73,78,82,87,92,98,104,110,117,123,131,139,147,156,165,175,185,196,208,220,233,247,262,277,294,311,330,349,370,392,415,440,466,494,523,554,587,622,659,698,740,784,831,880,932,988,1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976,2093,2217,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,4186,4435,4699,4978,5274,5588,5920,6272,6645,7040,7459,7902,8372,8870,9397,9956,10548,11175,11840,12544};
    size_t count = 0;
    uint16_t frequency[2] = {};
    for (size_t i=0; i<values;) {
        uint16_t command = pgm_read_word(score+i++);
        if (command == 0xf0 || command == 0xe0) {
            if (!count || i != values) { audio_result(-EINVAL); return; }
            if (command == 0xe0) {
                const indicator_buzzer_melody melody{owned_notes, static_cast<uint8_t>(count)};
                audio_result(mbs_indicator_buzzer_play_owned_repeat(INDICATOR_SOURCE_SYSTEM, &melody, &tone_token));
            } else { audio_submit(count); }
            return;
        }
        if (command == 0x91 || command == 0x81) {
#if !MESHBUS_ARDUBOY_COMPATIBILITY
            audio_result(-ENOTSUP); return;
#else
            if (command == 0x91) { printk("[arduboy] legacy two-channel score reduced to mono\n"); }
#endif
        }
        if (command == 0x90 || command == 0x91) {
            if (i == values) { audio_result(-EINVAL); return; }
            uint16_t note = pgm_read_word(score+i++);
            if (note >= 128) { audio_result(-ERANGE); return; }
            frequency[command & 1] = frequencies[note];
        } else if (command == 0x80 || command == 0x81) { frequency[command & 1] = 0; }
        else if (command < 0x80) {
            if (i == values) { audio_result(-EINVAL); return; }
            // Wide low words retain the legacy ports' explicit 260-ms value.
            uint16_t duration = static_cast<uint16_t>((command << 8) | pgm_read_word(score+i++));
            if (!duration) { audio_result(-EINVAL); return; }
            if (count == max_tone_notes) { audio_result(-E2BIG); return; }
            owned_notes[count++] = {frequency[0] ? frequency[0] : frequency[1], duration};
        } else { audio_result(-ENOTSUP); return; }
    }
    audio_result(-EINVAL);
}
