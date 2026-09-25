/* SPDX-License-Identifier: Apache-2.0 */
#include <zephyr/llext/symbol.h>
#include <meshbus_arduboy/runtime.hpp>
#include <meshbus_arduboy/runtime.hpp>
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <meshbus_arduboy/sketch.hpp>
#ifdef MESHBUS_ARDUBOY_TEST_STARTUP_RETRY
static void probe_setup()
{
    setup();
    printk("[bounce] setup frame published; owner waits 2000 ms\n");
    k_sleep(K_MSEC(2000));
    printk("[bounce] setup wait finished\n");
}
#endif
extern "C" void bounce_app_main(void *args)
{
#ifdef MESHBUS_ARDUBOY_TEST_STARTUP_RETRY
    const meshbus::arduboy::SketchConfig config{"bounce", probe_setup, loop};
#else
    const meshbus::arduboy::SketchConfig config{"bounce", setup, loop};
#endif
#ifdef MESHBUS_ARDUBOY_TEST_STARTUP_RETRY
    int failed = meshbus::arduboy::run_sketch(nullptr, config);
    printk("[bounce] invalid-host retry probe: %d\n", failed);
    if (failed != -EINVAL) { return; }
#endif
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(bounce_app_main);
