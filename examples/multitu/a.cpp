/* SPDX-License-Identifier: Apache-2.0 */
#include "common.hpp"
#include <EEPROM.h>
#include <meshbus_arduboy/eeprom_runtime.hpp>
#include <zephyr/sys/printk.h>
namespace { int local = 7; }
static int same_name = 11;
struct Lifetime {
    Lifetime() { ++shared_counter(); printk("[multitu] constructor A\n"); }
    ~Lifetime() { printk("[multitu] destructor A count=%d\n", shared_counter()); }
};
static Lifetime lifetime;
__attribute__((weak)) int weak_choice() { return 1; }
int part_a()
{
    EEPROM.update(16, 42);
    ++shared_counter();
    return local + same_name;
}
