/* SPDX-License-Identifier: Apache-2.0 */
#include <zephyr/llext/symbol.h>
#include <meshbus_arduboy/runtime.hpp>
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
static Arduboy2 board;
static uint8_t phase, frames;
static uint32_t first;
static const uint8_t rates[] = {15, 30, 60};
static void setup_timing()
{
    board.clear(); board.print("DELAY 3000 ms"); board.display();
    uint32_t before = millis(); delay(3000);
    printk("[timing] requested_ms=3000 actual_ms=%u\n", millis() - before);
    board.setFrameRate(rates[0]);
}
static void loop_timing()
{
    if (!board.nextFrame()) { return; }
    if (frames++ == 0) { first = millis(); }
    board.clear(); board.print("FPS "); board.print(rates[phase]);
    board.setCursor(0, 10); board.print("FRAME "); board.print(frames); board.display();
    if (frames < 31) { return; }
    printk("[timing] fps=%u intervals=30 elapsed_ms=%u\n", rates[phase], millis() - first);
    frames = 0;
    if (++phase < 3) { board.setFrameRate(rates[phase]); return; }
    board.clear(); board.print("DELAY 60000 ms"); board.setCursor(0, 10); board.print("Long Back exits"); board.display();
    uint32_t before = millis(); printk("[timing] long-delay start_ms=%u\n", before);
    delay(60000);
    printk("[timing] long-delay actual_ms=%u cancelled=%u\n", millis() - before,
           meshbus::arduboy::exit_requested() ? 1U : 0U);
    phase = 0; board.setFrameRate(rates[0]);
}
extern "C" void timing_app_main(void *args)
{
    const meshbus::arduboy::SketchConfig config{"timing", setup_timing, loop_timing};
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(timing_app_main);
