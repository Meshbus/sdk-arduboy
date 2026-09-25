/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MESHBUS_ARDUBOY_ARDUINO_PRINT_H_
#define MESHBUS_ARDUBOY_ARDUINO_PRINT_H_
#include <Arduino.h>

class Print {
public:
    virtual size_t write(uint8_t c) = 0;
    size_t print(const char *text) {
        size_t count = 0;
        if (text != nullptr) while (*text) count += write(static_cast<uint8_t>(*text++));
        return count;
    }
    size_t print(const __FlashStringHelper *text) {
        return print(reinterpret_cast<const char *>(text));
    }
    size_t print(char c) { return write(static_cast<uint8_t>(c)); }
    size_t print(unsigned long value) {
        char buffer[sizeof(value) * 3 + 1];
        char *end = buffer + sizeof(buffer) - 1;
        *end = 0;
        do { *--end = static_cast<char>('0' + value % 10); value /= 10; } while (value);
        return print(end);
    }
    size_t print(long value) {
        if (value < 0) {
            size_t count = write('-');
            return count + print(0UL - static_cast<unsigned long>(value));
        }
        return print(static_cast<unsigned long>(value));
    }
    size_t print(int value) { return print(static_cast<long>(value)); }
    size_t print(unsigned int value) { return print(static_cast<unsigned long>(value)); }
    size_t print(unsigned char value) { return print(static_cast<unsigned long>(value)); }
};
#endif
