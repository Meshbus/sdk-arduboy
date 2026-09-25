/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdint.h>
inline int &shared_counter() { static int count; return count; }
int part_a();
int part_b();
int weak_choice();
