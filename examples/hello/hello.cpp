/* SPDX-License-Identifier: Apache-2.0 */
#include <zephyr/llext/symbol.h>
#include <Arduboy2.h>
#include <meshbus_arduboy/runtime.hpp>

static Arduboy2 board;
static int16_t x = 60;

static void setup()
{
    board.begin();
    board.setFrameRate(30);
}

static void loop()
{
    if (!board.nextFrame()) {
        return;
    }
    board.pollButtons();
    if (board.pressed(LEFT_BUTTON) && x > 0) {
        --x;
    }
    if (board.pressed(RIGHT_BUTTON) && x < 120) {
        ++x;
    }
    if (board.justPressed(A_BUTTON)) {
        x = 60;
    }
    board.clear();
    board.setCursor(0, 0);
    board.print("Hello Arduboy");
    board.fillRect(x, 32, 8, 8, WHITE);
    board.display();
}

extern "C" void hello_app_main(void *args)
{
    meshbus::arduboy::SketchConfig config{"hello", setup, loop};
    config.save_id = "sdk_ab_hello";
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(hello_app_main);
