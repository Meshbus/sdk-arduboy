/* SPDX-License-Identifier: Apache-2.0 */
#include <ArduboyFX.h>
#include <arduboy-resource.hpp>
#include <zephyr/llext/symbol.h>
#include <zephyr/sys/printk.h>
extern void setup();
extern void loop();
extern int8_t playerX, playerY;
static void observed_loop() {
    static int x = -999, y = -999;
    loop();
    if (x != playerX || y != playerY) {
        x = playerX; y = playerY;
        printk("[isojourn] position=%d,%d FX rc=%d reads=%u decodes=%u\n",
            x, y, FX::lastError(), FX::readCount(), FX::decodeCount());
    }
}
extern "C" void isojourn_main(void *args) {
    meshbus::arduboy::SketchConfig config{"isojourn", setup, observed_loop};
    config.resources = &arduboy_resource;
    int rc = meshbus::arduboy::run_sketch(args, config);
    printk("[isojourn] complete rc=%d FX rc=%d reads=%u decodes=%u\n",
        rc, FX::lastError(), FX::readCount(), FX::decodeCount());
}
LL_EXTENSION_SYMBOL(isojourn_main);
