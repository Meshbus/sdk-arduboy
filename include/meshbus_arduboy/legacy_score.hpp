/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdint.h>
#include <stddef.h>
namespace meshbus::arduboy {
// Version 1 of the historical Meshbus score format. Arduino byte stays 8-bit.
using LegacyScoreWordV1 = uint16_t;
struct LegacyScoreSpanV1 { const LegacyScoreWordV1 *words; size_t count; };
void register_legacy_scores_v1(const LegacyScoreSpanV1 *scores, size_t count);
}
