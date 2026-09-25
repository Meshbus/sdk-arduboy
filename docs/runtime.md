# Runtime contracts

Use [Getting started](getting-started.md) for your first build and the
[API compatibility table](api-compatibility.md) when porting a game.

## EEPROM ownership and commit

`EepromFile::init`, `reset`, and `load` run before starting game callbacks.
The mirror passed to `init` is borrowed until shutdown: access it only through
`read`, `write`, `update`, or their block variants after initialization. Those
operations serialize on a short semaphore-protected critical section. A block
update is one mutation. Only the persistence owner calls `flush`; concurrent
flushes or independent objects sharing a save ID are unsupported.

Flush uses one bounded 1104-byte load/flush scratch buffer per MBA, included in its
loader memory reservation, and accepts EEPROM mirrors up to 1024 bytes. It
copies the mirror and generation under the lock, then releases the lock before
filesystem I/O. A concurrent flush returns EBUSY and preserves pending writes. The
object additionally stores a semaphore and a 32-bit generation counter. A
successful commit only acknowledges that generation. The runner retries failed
periodic saves with capped backoff (at most eight skipped 250 ms cycles) and
forces a final attempt after game callbacks have stopped.

Commit writes and syncs a same-directory `.tmp` file, checks close, and renames
over the previously committed save. The target filesystem backend must provide
atomic same-directory replacement semantics. Validate that contract for each
board/firmware configuration. Failure
removes only the temporary file. Native fault tests validate the sequencing;
they do not constitute physical power-loss testing.

## Program-memory readers

`pgm_read_word` and `pgm_read_dword` read 16/32-bit little-endian byte resources
at any alignment. `pgm_read_ptr` copies a native target pointer with `memcpy`;
a serialized 32-bit address is a number, not a host pointer. The legacy
`Arduboy.h` adapter alone preserves word reads for Hopper's byte-image pointer
tables and Hollow's string pointer tables. New games should use `pgm_read_ptr`.

## Input consumption

Arduboy2 button edges follow the pinned upstream reference: a composite
`justPressed(A | B)` means none of the selected buttons was down and at least
one now is; it does not require both buttons. `justReleased` is the reverse.
`pressed(A | B)` requires all selected buttons.

The runner latches PRESS/RELEASE transitions and synthesizes CLICK into one
consumable down frame followed by release. Multiple clicks between frames are
queued (255 per action, saturation reported); a raw tap and its CLICK before
consumption coalesce. Each catch-up tick consumes separately. Enter/leave and
exit clear held and queued state. The exit action mask is configurable; the
default exit mask consumes `ZUI_ACTION_CANCEL`, reserving that action from games.
The physical gesture (often long Back) comes from the target input configuration.

## Single-owner sketch runtime

`run_sketch(args, SketchConfig{id, setup, loop})` binds one AppContext for the
session. The CMake `RUNTIME` option links its implementation once and defines
`MESHBUS_ARDUBOY_RUNTIME=1` for all sources. See [building](building.md).
The fullscreen runner provides a separate callback-based interface.

The existing MBA application thread owns setup/loop, clock/random state, drawing,
and EEPROM operations. Display attaches before setup. ZUI callbacks only queue
input and copy the submitted frame. Publication copies 1024 bytes under a short
semaphore lock; the UI copies into its own 1024-byte staging frame, releases the
lock, then draws. No state lock spans the display driver, filesystem I/O or sleep.
The additional submitted/UI snapshots cost 2048 bytes, charged in MBA BSS; the
1024-byte drawing and EEPROM mirrors also live in BSS, avoiding the shared stack.
The runtime creates no thread. Exit stops game execution, detaches ZUI, stops the
tone adapter and attempts the final EEPROM commit before returning to Desktop.

Runtime calls during global construction or after exit report unavailable state;
constructors must limit themselves to local state initialization. RuntimeHooks
permit explicit time/input adaptation; omitted hooks use the Meshbus host clock
and ZUI action adapter. Frame-gated sketches use their own nextFrame clock.
Set frame_gated=false only
for callbacks that deliberately omit nextFrame and use SketchConfig.fps cadence.

## Runtime timebase

Runtime builds route millis/micros, delay/delayShort and the Arduboy2 frame API
through AppContext. The pinned upstream clock uses floor(1000/FPS) milliseconds
and returns false once immediately after a rendered frame. Frame count wraps at
16 bits; elapsed time uses a 32-bit unsigned difference, including millis wrap.
Rates 15/30/60 therefore request periods of 66/33/16 ms. Rate zero is rejected;
this subset keeps everyXFrames(0) defined as true for legacy compatibility.

The executor waits until the next due frame; it does not impose an additional
fixed ZUI tick. Input pulses are consumed only on accepted frames. Delay waits
on the existing exit semaphore, in bounded chunks, without holding the bridge
lock. The configured exit action wakes it; the game must return from its callback
to finish exit.
Constructors cannot wait on an unbound runtime. RuntimeHooks override the game
clock explicitly; callers must keep those overrides coherent with elapsed time.
`examples/timing` measures delays, frame-rate changes and cancellation on the selected target.

## Submitted frame lifetime

In runtime builds, display(false) copies the drawing buffer to the submitted
frame. display(true) performs the same copy and then clears only the drawing
buffer. Subsequent source writes and periods without display calls cannot change
the submitted image. A separate UI staging copy avoids holding the bridge lock
across zui_draw_framebuffer or the display driver.

The submitted and staging buffers cost 1024 bytes each in extension BSS. They
are covered by the MBA loader reservation and released with it; there is no
additional fallible snapshot allocation after loading. Reservation failure is a
loader failure before setup; ZUI allocation failures use the rollback path.
Native tests stress 2000 publications against a concurrent UI reader.
`examples/snapshot` permits deterministic, input-driven framebuffer verification on the selected target.


## Fullscreen EEPROM integration

The fullscreen runner can own Arduino EEPROM compatibility through
`FullscreenConfig::eeprom` when the app is built with
`MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM=1`. The app supplies a save id, data
buffer callback, and buffer size; the runner loads the save before app setup,
periodically flushes dirty data while running, and detaches the active EEPROM
on exit. Fullscreen runner apps without this define omit its EEPROM/FS dependencies.
`RUNTIME` integrations include shared EEPROM support automatically.
`EepromFile::init` uses raw-file storage. This
matches the byte-addressed behavior Arduboy games expect while avoiding changes
to upstream save-game code. Save ids are limited to ASCII letters, digits,
underscore, and hyphen; paths are always generated inside `/extra/saves` with a
`.dat` extension. `run_sketch` integrations use a versioned `.sav` envelope
with explicit migration and corruption protection; see [save formats](saves.md).
