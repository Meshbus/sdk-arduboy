/* SPDX-License-Identifier: Apache-2.0 */
#include <Arduboy2.h>
#include <meshbus_arduboy/runtime.hpp>
#include <arduboy-resource.hpp>
#include <zephyr/llext/symbol.h>
#include <zephyr/sys/printk.h>

using namespace meshbus::arduboy;
Arduboy2 board;
uint8_t scene;
bool valid;
void render()
{
    int rc = resource_read(scene * 1024U, board.getBuffer(), 1024);
    uint8_t level[8]{};
    if (rc == 0) { rc = resource_read(2048, level, sizeof(level)); }
    board.fillRect(0, 0, 128, 12, BLACK);
    board.setCursor(0, 0); board.print(valid && rc == 0 ? "RESOURCE PASS " : "RESOURCE FAIL ");
    board.print(scene);
    for (unsigned i = 0; i < 5; ++i) { board.fillRect(level[i], 35, 8, 8, WHITE); }
    board.display();
    printk("[resource_fixture] scene=%u length=%u rc=%d cross_boundary=%u\n",
           scene, resource_length(), rc, valid);
}
void setup()
{
    board.begin(); board.setFrameRate(30);
    uint8_t crossing[20]{};
    int rc = resource_read(250, crossing, sizeof(crossing));
    valid = rc == 0;
    for (unsigned i = 0; i < sizeof(crossing); ++i) {
        uint8_t expected = ((250 + i) % 128) % 16 < 8 ? 0x55 : 0xaa;
        valid = valid && crossing[i] == expected;
    }
    render();
}
void loop()
{
    if (!board.nextFrame()) { return; }
    board.pollButtons();
    if (board.justPressed(A_BUTTON)) { scene ^= 1; render(); }
}
extern "C" void resource_fixture_main(void *args)
{
    SketchConfig config{"resource_fixture", setup, loop};
    config.resources = &arduboy_resource;
    int rc = run_sketch(args, config);
    printk("[resource_fixture] complete rc=%d\n", rc);
}
LL_EXTENSION_SYMBOL(resource_fixture_main);
