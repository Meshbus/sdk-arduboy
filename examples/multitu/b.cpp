/* SPDX-License-Identifier: Apache-2.0 */
#include "common.hpp"
#include <EEPROM.h>
#include <meshbus_arduboy/eeprom_runtime.hpp>
namespace { int local = 13; }
static int same_name = 17;
int weak_choice() { return 2; }
int part_b()
{
    ++shared_counter();
    return local + same_name + EEPROM.read(16);
}
