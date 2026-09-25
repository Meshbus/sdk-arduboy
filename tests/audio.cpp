#include <cassert>
#define MESHBUS_ARDUBOY_TEST_NO_MAIN
#include "storage.cpp"
#define MESHBUS_ARDUBOY_RUNTIME 1
#include "../src/runtime.cpp"
static ArduboyTones sound(Arduboy2Audio::enabled);
static void setup_audio()
{
    assert(Arduboy2Audio::enabled());
    sound.tone(440, 100);
    assert(sound.playing());
    kernel_fixture::ticks += 3125;
    assert(!sound.playing());
    sound.tone(660, 0); assert(sound.playing());
    Arduboy2Audio::off(); assert(!sound.playing());
    Arduboy2Audio::saveOnOff();
    assert(meshbus_arduboy_eeprom_read(2) == 0);
    assert(meshbus_arduboy_eeprom_read(16) == 0xff);
    Arduboy2Audio::on();
    uint16_t pairs[] = {440, 100, 660, 100, TONES_END};
    sound.tones(pairs); assert(sound.playing());
    pairs[0] = 1; // Caller may release/change the source after the call.
    assert(indicator_fixture::notes[0].freq_hz == 440);
    kernel_fixture::ticks += 6250;
    assert(!sound.playing());
    indicator_fixture::ready = false; sound.tone(440, 100);
    assert(!sound.playing() && meshbus_arduboy_audio_status() == -ENODEV);
    indicator_fixture::ready = true;
    uint16_t repeat[] = {440, 100, TONES_REPEAT};
    sound.tones(repeat);
    assert(!sound.playing() && meshbus_arduboy_audio_status() == -ENOTSUP);
    sound.tone(660, 1000); assert(sound.playing());
}
static void loop_audio() {}
static void setup_disabled()
{
    assert(!Arduboy2Audio::enabled());
    sound.tone(440, 100); assert(!sound.playing());
    assert(meshbus_arduboy_audio_status() == -EACCES);
    Arduboy2Audio::on();
    uint16_t incomplete[] = {440, 100};
    sound.tones(incomplete, 2);
    assert(!sound.playing() && meshbus_arduboy_audio_status() == -EINVAL);
    sound.tone(440, 0); assert(sound.playing());
    ++indicator_fixture::token; // A host notification preempts the game.
    assert(!sound.playing());
    sound.noTone(); assert(indicator_fixture::active); // Preserve the newer owner.
    indicator_fixture::active = false;
}

int main()
{
    indicator_fixture::ready = true;
    zui_host host{}; mbs_desktop_app_args args{&host};
    meshbus::arduboy::SketchConfig config{"audio_test", setup_audio, loop_audio};
    assert(meshbus::arduboy::run_sketch(&args, config) == 0);
    assert(!indicator_fixture::active);
    assert(files["/extra/saves/audio_test.sav"][82] == 0);
    config.setup = setup_disabled;
    assert(meshbus::arduboy::run_sketch(&args, config) == 0);
}
