# API compatibility for game ports

This table describes the SDK subset, not full upstream compatibility.
`ARDUBOY_LIB_VER` selects upstream source branches (50200 for Arduboy2, 10101 for
legacy Arduboy); it is not a support certificate. Start new ports with `RUNTIME`
and strict diagnostics. Headers are the authoritative overload/signature list.

| Surface | Supported here | Limits / migration |
| --- | --- | --- |
| Arduino time/random | `millis`, `micros`, `delay`, `randomSeed`, both `random` forms | Runtime-bound clocks; no general Arduino core or peripheral ownership |
| Program memory | `PROGMEM`, `F`, `PSTR`, string/memory shims, `pgm_read_*` | Native memory; numeric word/dword reads are little-endian, pointers require `pgm_read_ptr` |
| Arduboy2 lifecycle | `begin`, `boot`, `clear`, `getBuffer`, `display` | `begin` clears the framebuffer; AVR startup hardware routines are not reproduced |
| Frame timing | `setFrameRate`, `nextFrame`, `nextFrameDEV`, `frameCount`, `everyXFrames`, `cpuLoad`, `delayShort` | Integer-ms periods, 16-bit count; `everyXFrames(0)` is true; constructors cannot call runtime APIs |
| Buttons | `pollButtons`, `pressed`, `justPressed`, `justReleased` | Six logical game actions; CANCEL reserved for exit by default; composite edge semantics in [runtime](runtime.md) |
| Geometry | Pixel, horizontal/vertical/arbitrary line, rectangle, filled rectangle, circle, fill screen | Logical monochrome 128×64 framebuffer; other upstream shapes/overloads may be absent |
| Bitmaps | `drawBitmap`, Sprites overwrite/self-mask/plus-mask/external-mask | AVR assembly and other Sprites variants are not implemented; validate asset lengths in the port |
| Text / Print | `setCursor`, `write`, string/flash-string, char and decimal integer `print` | Fixed 5×7 font with 6×8 advance; no general `println`, floating formatting, bases, text scaling or complete Stream API |
| EEPROM | `read`, `write`, `update`, `get`, `put`, `length` | 1024 bytes; typed operations copy native object representation; settings use bytes 0–15 |
| Audio | `Arduboy2Audio`, ArduboyTones one/two/three-note and bounded score calls | Mono tone adapter, up to 32 pairs, host availability/mute/preemption; no volume control or `TONES_REPEAT` |
| Legacy Arduboy | Bounded Hopper/Hollow interfaces, registered legacy score spans | Explicit old-score formats; polyphony only reduces to mono in compatibility mode; see [legacy](legacy.md) |
| FX | `begin`, `readDataArray`, normal/masked `drawBitmap`, `display` | Read-only resources; no save regions, flash carts, grayscale or streamed audio; see [FX](fx.md) |
| Sketch preprocessing | Main `.ino` first, other `.ino`/`.pde` alphabetically, separate `.cpp`, bounded prototype generation | No general C++ parser; complex declarations need explicit headers / `.cpp`; see [building](building.md) |
| GPIO/SPI/RGB/system startup | No physical mapping | Strict diagnostics reject mapped unsupported calls; compatibility mode preserves documented no-op/echo placeholders |

Absent methods fail normal compilation. Some bounded runtime operations return
errors or request exit rather than being diagnosed at compile time. Merely
including an unsupported interface need not fail compilation.

## Strict versus compatibility mode

`meshbus_arduboy_llext_add_app(... RUNTIME ...)` defaults to strict mode.
`COMPATIBILITY` is an explicit per-port choice allowing documented legacy
placeholders. It does not supply missing peripherals or turn partial support
into correct gameplay. Review each diagnostic before opting in. See
[capabilities](capabilities.md) for `REQUIRES` and the machine-readable report.

## AVR to host migration checklist

1. Keep upstream sources pinned and isolate bridge/platform changes.
2. Use fixed-width fields for serialized data: AVR `int` is commonly 16-bit,
   while other targets may use 32-bit `int`; verify the selected target ABI.
   Arduino `byte` remains 8-bit here.
3. Read native pointer tables with `pgm_read_ptr`; do not treat a serialized
   16/32-bit address as a native pointer.
4. Preserve a stable save ID, reserve bytes 0–15, and explicitly migrate layouts.
   `EEPROM.get/put` does not normalize padding, endian order, or pointer values.
5. Return regularly from setup/loop and use cancellable SDK delays. CPU-bound
   loops must check `exit_requested()` to cooperate with stop requests.
6. Validate required imports, package memory bounds, and each used API on the
   host first; then record device rendering, controls, audio, and save/reload.
