#include <cassert>
#include <meshbus_arduboy/clock.hpp>
int main() {
 const uint64_t values[] = {0, 1, 31, 125, UINT32_MAX, UINT64_C(134217728000), UINT64_C(987654321234)};
 for (auto value : values) {
     assert(meshbus::arduboy::ticks_to_millis(value, 31250) == static_cast<uint32_t>(value * 1000 / 31250));
 }
 meshbus::arduboy::FrameClock clock;
 assert(!clock.next(0) && !clock.next(15));
 assert(clock.next(16) && clock.count() == 1);
 assert(!clock.next(16) && !clock.next(31));
 assert(clock.next(32) && clock.count() == 2); assert(!clock.next(32));
 assert(clock.set_rate(15)); assert(!clock.next(97) && clock.next(98));
 assert(!clock.next(98)); assert(clock.set_rate(30)); assert(clock.next(131));
 assert(!clock.next(131)); assert(clock.set_rate(60)); assert(clock.next(147));
 assert(!clock.set_rate(0));
 meshbus::arduboy::FrameClock wrapped;
 assert(wrapped.next(UINT32_MAX - 5)); assert(!wrapped.next(UINT32_MAX - 5));
 assert(!wrapped.next(9)); assert(wrapped.next(10));
 meshbus::arduboy::FrameClock count;
 for (uint32_t i=1; i<=65536; ++i) { assert(count.next(i*16)); assert(!count.next(i*16)); }
 assert(count.count() == 0);

}
