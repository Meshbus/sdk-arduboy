/* SPDX-License-Identifier: Apache-2.0 */
#include "common.hpp"
#include <Arduboy2.h>
#include <meshbus_arduboy/runtime.hpp>
#include <zephyr/llext/symbol.h>
#include <zephyr/sys/printk.h>
static Arduboy2 board;
static void setup_game()
{
    int a = part_a(), b = part_b(), weak = weak_choice(), shared = shared_counter();
    bool pass = a == 18 && b == 72 && weak == 2 && shared == 3;
    printk("[multitu] a=%d b=%d weak=%d shared=%d pass=%u\n", a,b,weak,shared,pass);
    board.clear(); board.setCursor(0,0); board.print(pass ? "Multi TU PASS" : "Multi TU FAIL"); board.display();
}
static void loop_game() {}
extern "C" void multitu_app_main(void *args)
{
    meshbus::arduboy::SketchConfig config{"multitu", setup_game, loop_game};
    config.frame_gated = false; config.save_id = "sdk_ab_multitu_20260913";
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(multitu_app_main);
