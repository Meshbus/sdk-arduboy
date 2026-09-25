# Minimal game

Build `hello` using the command in the [example index](../README.md). The screen
shows `Hello Arduboy` and an 8×8 square. Left/right move it, A centers it, and long
Back exits. The runtime uses the separate save ID `sdk_ab_hello`; this example
does not write game-progress fields.

`hello.cpp` supplies setup/loop and an exported Desktop entry point. `RUNTIME`
links the shared implementation once; `nextFrame()` gates updates at 30 FPS.
The constructor initializes only local state. `run_sketch` owns host attachment,
input, clocks, saves and cleanup. No AVR initialization code is needed.

To use this as a template, copy the directory, pass an absolute
`MESHBUS_ARDUBOY_SDK_DIR`, and rename the CMake project/target, YAML id/entry point,
exported function and save ID consistently. Reuse an existing save ID only when
its layout and migration are intentional. See [save schemas](../../docs/saves.md).
