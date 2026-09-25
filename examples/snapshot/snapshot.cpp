/* SPDX-License-Identifier: Apache-2.0 */
#include <zephyr/llext/symbol.h>
#include <meshbus_arduboy/runtime.hpp>
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
static Arduboy2 board;
static uint8_t stage;
static void publish(uint8_t pattern, bool clear, uint8_t overwrite)
{
    auto *buffer = board.getBuffer(); memset(buffer, pattern, 1024);
    board.display(clear);
    bool cleared = true;
    if (clear) for (unsigned i=0; i<1024; ++i) cleared &= buffer[i] == 0;
    memset(buffer, overwrite, 1024);
    printk("[snapshot] published=%02x clear=%u clear_check_passed=%u source_now=%02x\n",
           pattern, clear ? 1U : 0U, cleared ? 1U : 0U, overwrite);
}
static void setup_snapshot() { publish(0x55, true, 0xaa); }
static void loop_snapshot()
{
    board.pollButtons();
    if (!board.justPressed(A_BUTTON)) { return; }
    if (stage == 0) { publish(0x0f, true, 0xf0); ++stage; }
    else if (stage == 1) { publish(0x33, false, 0xcc); ++stage; }
}
extern "C" void snapshot_app_main(void *args)
{
    meshbus::arduboy::SketchConfig config{"snapshot", setup_snapshot, loop_snapshot, 30};
    config.frame_gated = false;
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(snapshot_app_main);
