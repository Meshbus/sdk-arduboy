/* SPDX-License-Identifier: Apache-2.0 */
#include <meshbus_arduboy/runtime.hpp>
#include <meshbus_arduboy/sketch.hpp>
#include <zephyr/llext/symbol.h>
extern "C" void sketch_fixture_app_main(void *args)
{
    meshbus::arduboy::SketchConfig config{"sketch_fixture", setup, loop};
    config.frame_gated = false;
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(sketch_fixture_app_main);
