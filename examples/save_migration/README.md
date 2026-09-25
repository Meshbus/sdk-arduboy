# Explicit save migration example

This example uses only `/extra/saves/sdk_ab_save_20260913.sav`.
Provision a version-1 envelope with schema 1 and a 1024-byte payload before
launching to exercise the [migration callback](migrations.hpp): byte 16 is a
little-endian u16 score and
byte 18 a little-endian i16 position. The destination schema 2 uses u32/i32
at bytes 16 and 20. A increments the score; long Back exits and flushes.
Restarting the example displays the committed value. See
[the format contract](../../docs/saves.md) for CRC, identity and failure rules.

The Hopper/Hollow callbacks in `migrations.hpp` are explicitly scoped to the
pinned legacy ports and are tested against copied raw fixtures and nonempty
synthetic records. They never delete the raw source.

Build commands and package outputs are in the [example index](../README.md).
