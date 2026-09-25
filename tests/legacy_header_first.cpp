#define MESHBUS_ARDUBOY_COMPATIBILITY 1
#include <Arduboy.h>
#include <Arduboy2.h>
#include <Arduino.h>
static_assert(sizeof(byte)==1);
static_assert(ARDUBOY_LIB_VER==50200);
int main() {}
