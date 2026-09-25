# Legacy Arduboy ports

`Arduboy.h` uses the same runtime clock, display, input and audio ownership as
Arduboy2 when built with RUNTIME. Without RUNTIME, callback entry points are
available in explicit compatibility mode. Arduino `byte` is always
uint8_t, including either header order. When both Arduboy headers are included,
ARDUBOY_LIB_VER reports the Arduboy2 version; the legacy-only header retains its
original version marker.

The pinned Hopper/Hollow ports use uint16 score words, including an original
260-ms low duration word. They explicitly scope the `LegacyScoreWordV1` type
to upstream score declarations/parameters. No global Arduino type is widened.
The ports register exact array spans with `register_legacy_scores_v1`; unknown
pointers, missing terminators, invalid notes and unsupported commands fail
before any prefix plays. The span table and scores must remain valid until
exit. Decoded notes are copied into runtime-owned storage, at most 32 notes.
Channel0 supports note-on90, note-off80, positive waits, F0stop and E0repeat.
Repeated finite melodies retain one host ownership token until stop/preemption;
they never reacquire ownership after a host notification preempts them.

Hollow's collision uses channel1 too. Strict mode rejects this polyphony;
explicit compatibility mode selects channel0 when both sound, or channel1
otherwise, and logs the mono reduction. This is the current audio adapter's
mono-playback limit; additional board audio hardware does not enable full AVR
two-channel synthesis through this adapter. AVR pin/timer setup and
startup LED behavior are compatibility no-ops, except timer3
on/off maps to the runtime audio setting. The configured CANCEL action exits
the runtime; its physical gesture depends on the target input mapping.

The production `legacy_saves.hpp` adapter validates pinned signatures and
record checksums, preserves game bytes, and moves byte0 audio into reserved
runtime byte2. Only explicitly selected raw .dat identities migrate to .sav;
old files remain untouched. Device migration checks must use separate identities
and backed-up, checksum-valid synthetic progress, preserving user saves. See
[testing](testing.md) for public evidence and device acceptance requirements.
