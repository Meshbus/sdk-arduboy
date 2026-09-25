#include <cassert>
#include <meshbus_arduboy/input.hpp>
using meshbus::arduboy::InputLatch;
int main() {
 InputLatch latch;
 latch.press(1); latch.release(1); // Entire physical tap between two frames.
 assert(latch.consume() == 1); assert(latch.consume() == 0);
 latch.click(2); latch.click(2);
 assert(latch.consume() == 2); assert(latch.consume() == 0);
 assert(latch.consume() == 2); assert(latch.consume() == 0);
 latch.press(1); latch.release(1); latch.click(1); // No duplicate raw tap.
 assert(latch.consume() == 1); assert(latch.consume() == 0); assert(latch.consume() == 0);
 latch.press(3); assert(latch.consume() == 3); assert(latch.consume() == 3);
 latch.release(1); assert(latch.consume() == 2); latch.reset(); assert(latch.consume() == 0);
 for (int i=0; i<255; ++i) assert(latch.click(1));
 assert(!latch.click(1)); // Saturation is observable, never wraps away pending input.
}
