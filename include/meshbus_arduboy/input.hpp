/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_INPUT_HPP_
#define MESHBUS_ARDUBOY_INPUT_HPP_
#include <stdint.h>

namespace meshbus::arduboy {
/* The caller serializes event production and frame consumption. */
class InputLatch {
public:
    void reset() { *this = InputLatch{}; }
    bool press(uint32_t mask) {
        bool accepted = enqueue(mask & ~held_);
        held_ |= mask; raw_seen_ |= mask;
        return accepted;
    }
    void release(uint32_t mask) { held_ &= ~mask; }
    bool click(uint32_t mask) {
        // A raw tap followed by CLICK before consumption is one tap.
        uint32_t synthetic = mask & ~raw_seen_;
        raw_seen_ &= ~mask;
        return enqueue(synthetic);
    }
    uint32_t consume() {
        uint32_t result = held_;
        for (unsigned i=0; i<32; ++i) {
            uint32_t bit = uint32_t(1) << i;
            if (pending_[i] == 0) { continue; }
            if (previous_ & bit) { result &= ~bit; }
            else { result |= bit; --pending_[i]; }
        }
        previous_ = result;
        raw_seen_ &= held_;
        return result;
    }
    uint32_t held() const { return held_; }
private:
    bool enqueue(uint32_t mask) {
        bool accepted = true;
        for (unsigned i=0; i<32; ++i) {
            if ((mask & (uint32_t(1) << i)) == 0) { continue; }
            if (pending_[i] == UINT8_MAX) { accepted = false; }
            else { ++pending_[i]; }
        }
        return accepted;
    }
    uint32_t held_ = 0, previous_ = 0, raw_seen_ = 0;
    uint8_t pending_[32] = {};
};
}
#endif
