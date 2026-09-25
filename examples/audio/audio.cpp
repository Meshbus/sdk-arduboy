/* SPDX-License-Identifier: Apache-2.0 */
#include <zephyr/llext/symbol.h>
#include <meshbus_arduboy/runtime.hpp>
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <meshbus_arduboy/eeprom_runtime.hpp>
#include <indicator/indicator.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
static Arduboy2 board;
static ArduboyTones sound(Arduboy2Audio::enabled);
static bool was_playing;
static void show_audio()
{
    board.clear(); board.setCursor(0,0);
    board.print("Audio: "); board.print(Arduboy2Audio::enabled() ? "ON\n" : "OFF\n");
    board.print("A: three notes\nB: toggle + save\nUP: stop\nLong Back: exit"); board.display();
}
static void setup_audio()
{
    printk("[audio] loaded enabled=%u setting=%u ready=%u\n", Arduboy2Audio::enabled(),
           meshbus_arduboy_eeprom_read(2), mbs_indicator_buzzer_is_ready());
    show_audio();
}
static void loop_audio()
{
    board.pollButtons();
    if (board.justPressed(A_BUTTON)) {
        sound.tone(440, 500, 660, 500, 880, 500);
        printk("[audio] requested 440/660/880Hz 500ms each status=%d playing=%u\n",
               meshbus_arduboy_audio_status(), sound.playing());
    }
    if (board.justPressed(B_BUTTON)) {
        if (Arduboy2Audio::enabled()) { Arduboy2Audio::off(); }
        else { Arduboy2Audio::on(); }
        Arduboy2Audio::saveOnOff(); show_audio();
        printk("[audio] saved enabled=%u setting=%u\n", Arduboy2Audio::enabled(), meshbus_arduboy_eeprom_read(2));
    }
    if (board.justPressed(UP_BUTTON)) { sound.noTone(); printk("[audio] stop playing=%u\n", sound.playing()); }
    bool playing = sound.playing();
    if (playing != was_playing) { printk("[audio] playing=%u at=%u\n", playing, millis()); was_playing = playing; }
}
extern "C" void audio_app_main(void *args)
{
    meshbus::arduboy::SketchConfig config{"audio", setup_audio, loop_audio, 60};
    config.frame_gated = false; config.save_id = "sdk_ab_audio_20260913";
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(audio_app_main);
