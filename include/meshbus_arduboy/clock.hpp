/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_CLOCK_HPP_
#define MESHBUS_ARDUBOY_CLOCK_HPP_
#include <stdint.h>
namespace meshbus::arduboy {
/* Low 32 bits of a quotient, without a 64-bit division runtime import. */
static inline uint32_t divide_u64_u32_low(uint64_t numerator, uint32_t divisor)
{
    uint32_t remainder = static_cast<uint32_t>(numerator >> 32) % divisor;
    uint32_t low = static_cast<uint32_t>(numerator), quotient = 0;
    for (int bit = 31; bit >= 0; --bit) {
        // Clock divisors are bounded below 2^31.
        remainder = (remainder << 1) | ((low >> bit) & 1U);
        quotient <<= 1;
        if (remainder >= divisor) { remainder -= divisor; quotient |= 1; }
    }
    return quotient;
}
static inline uint32_t ticks_to_millis(uint64_t ticks, uint32_t rate)
{
    return divide_u64_u32_low(ticks * 1000U, rate);
}
/* Arduboy2 bc460a2: integer millisecond periods and a post-render false pass.
 * Use a 32-bit time delta so waits over 255ms and millis rollover stay defined.
 */
class FrameClock {
public:
    bool set_rate(uint8_t rate) {
        if (rate == 0) { return false; }
        period_ = 1000U / rate;
        return true;
    }
    bool next(uint32_t now) {
        uint32_t elapsed = now - start_;
        if (rendered_) {
            last_duration_ = elapsed; rendered_ = false; return false;
        }
        if (elapsed < period_) { return false; }
        rendered_ = true; start_ = now; ++count_; return true;
    }
    uint32_t wait_ms(uint32_t now) const {
        if (rendered_ || now - start_ >= period_) { return 0; }
        return period_ - (now - start_);
    }
    uint16_t count() const { return count_; }
    void set_count(uint16_t count) { count_ = count; }
    uint8_t load() const {
        uint32_t value = last_duration_ > period_ * 255U / 100U ? 255U : last_duration_ * 100U / period_;
        return static_cast<uint8_t>(value);
    }
private:
    uint32_t start_ = 0, period_ = 16, last_duration_ = 0;
    uint16_t count_ = 0;
    bool rendered_ = false;
};

}
#endif
