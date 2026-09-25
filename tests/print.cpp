#include <string>
#include <climits>
#include <cassert>
#include <Print.h>
struct Buffer : Print {
 std::string value;
 size_t write(uint8_t c) override { value += static_cast<char>(c); return 1; }
};
int main() { Buffer out; out.print(INT_MIN); assert(out.value == std::to_string(INT_MIN));
 out.value.clear(); out.print(LONG_MIN); assert(out.value == std::to_string(LONG_MIN));
 out.value.clear(); out.print(ULONG_MAX); assert(out.value == std::to_string(ULONG_MAX));
 out.value.clear(); out.print(F("Score: ")); out.print(123U); assert(out.value == "Score: 123"); }
