#include <cassert>
#include <Arduboy2.h>
static uint8_t buttons;
extern "C" uint8_t meshbus_arduboy_buttons() { return buttons; }
int main() {
 Arduboy2Base a; a.pollButtons(); buttons = A_BUTTON; a.pollButtons();
 assert(a.justPressed(A_BUTTON | B_BUTTON));
 assert(a.justPressed(A_BUTTON) && !a.justPressed(B_BUTTON));
 buttons = A_BUTTON | B_BUTTON; a.pollButtons();
 assert(!a.justPressed(A_BUTTON | B_BUTTON) && a.justPressed(B_BUTTON));
 buttons = 0; a.pollButtons(); assert(a.justReleased(A_BUTTON | B_BUTTON));
 a.pollButtons(); assert(!a.justReleased(A_BUTTON | B_BUTTON));
}
