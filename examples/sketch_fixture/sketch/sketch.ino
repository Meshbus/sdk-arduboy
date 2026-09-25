/* SPDX-License-Identifier: Apache-2.0 */
#include <Arduboy2.h>
#include <zephyr/sys/printk.h>
#include "worker.hpp"
Arduboy2 board;
int sequence = 1;
extern int sequence_end;
void setup()
{
    drawLater();
}
