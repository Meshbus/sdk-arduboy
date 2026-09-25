/* SPDX-License-Identifier: Apache-2.0 */
#include <zephyr/llext/symbol.h>
#include "migrations.hpp"
#include <meshbus_arduboy/runtime.hpp>
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <EEPROM.h>
static Arduboy2 board;
static void show_save()
{
    uint8_t record[8]; EEPROM.get(16, record);
    uint32_t score = meshbus::arduboy::save_format::read32(record);
    uint32_t position = meshbus::arduboy::save_format::read32(record + 4);
    printk("[save_migration] score=%u position_bits=%08x audio=%u\n", score, position, EEPROM.read(2));
    board.clear(); board.setCursor(0, 0); board.print("Save migration v2\n");
    board.print(static_cast<unsigned long>(score)); board.print("\nA: increment score"); board.display();
}
static void setup_save() { show_save(); }
static void loop_save()
{
    board.pollButtons();
    if (!board.justPressed(A_BUTTON)) { return; }
    uint8_t record[4]; EEPROM.get(16, record);
    meshbus::arduboy::save_format::write32(record, meshbus::arduboy::save_format::read32(record) + 1);
    EEPROM.put(16, record); show_save();
}
extern "C" void save_migration_app_main(void *args)
{
    meshbus::arduboy::SketchConfig config{"save_migration", setup_save, loop_save, 30};
    config.frame_gated = false; config.save_id = "sdk_ab_save_20260913";
    config.save_schema = 2; config.migrate = save_examples::avr_to_c2;
    (void)meshbus::arduboy::run_sketch(args, config);
}
LL_EXTENSION_SYMBOL(save_migration_app_main);
