#include <cassert>
#include <cstring>
#include <Arduino.h>
int main() {
    const uint8_t bytes[] = {0xff, 0x34, 0x12, 0x78, 0x56};
    assert(pgm_read_word(bytes + 1) == 0x1234);
    const void *unaligned = bytes + 1;
    assert(pgm_read_word(unaligned) == 0x1234);
    assert(pgm_read_dword(unaligned) == 0x56781234);
    const uint8_t *pointer = bytes;
    uint8_t table[1 + sizeof(pointer)];
    memcpy(table + 1, &pointer, sizeof(pointer));
    assert(pgm_read_ptr(static_cast<const void *>(table + 1)) == bytes);
    assert(pgm_read_ptr(&pointer) == bytes);
    // Serialized 32-bit addresses remain numeric even on a 64-bit host.
    assert(pgm_read_dword(bytes + 1) == UINT32_C(0x56781234));
}
