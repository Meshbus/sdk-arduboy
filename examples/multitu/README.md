# Multi-TU lifecycle fixture

Build the three application sources independently with CMake RUNTIME SOURCES.
The display shows Multi TU PASS only if TU-local variables stay isolated,
EEPROM writes in A are visible in B, a strong function overrides the weak one,
and an inline-function static is shared with the global constructor.
The constructor logs before app entry; the destructor logs count=3 after exit.
Long Back returns to Desktop. Its dedicated save ID is sdk_ab_multitu_20260913.

Build commands and package outputs are in the [example index](../README.md).
