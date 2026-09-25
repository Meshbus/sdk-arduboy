# Save identity and schema

`SketchConfig.id` identifies a runtime for diagnostics. `save_id` defaults to it,
can be set independently, and must remain stable across display-name or release
version changes. IDs contain ASCII letters, digits, `_` or `-`; at most 46 bytes
fit the current 64-byte `/extra/saves/<id>.sav` path. `save_schema` is an explicit
nonzero application data-layout version, independent of the game release.

Runtime saves use versioned `.sav` envelopes through `EepromFile.init_versioned`.
`EepromFile.init` uses raw `.dat` files; migration preserves those source files.
Loading a failed, unsupported or unapproved migration disables commits. The
runtime aborts before game setup on load errors. `load_status()` distinguishes
fresh, loaded, legacy available, migrated, corrupt, wrong size/schema/identity,
migration failure and I/O error. A missing file is fresh; an invalid `.sav` never
falls back silently to `.dat`. A migration callback is an explicit opt-in, not
a request to guess the old layout. It receives source schema 0 for raw data.

## Version 1 envelope

All integers are unsigned little endian. No native struct is written to disk.
The file is exactly 80 bytes of header followed by 1–1024 bytes of payload.

| Offset | Bytes | Meaning |
| --- | --- | --- |
| 0 | 4 | ASCII `MASV` |
| 4 | 2 | Envelope version 1 |
| 6 | 2 | Header size 80 |
| 8 | 4 | Nonzero game schema |
| 12 | 4 | Payload byte length |
| 16 | 4 | Payload CRC32 |
| 20 | 4 | Reserved, zero |
| 24 | 48 | Save ID, NUL terminated and zero padded |
| 72 | 4 | CRC32 of header bytes 0–71 |
| 76 | 4 | Reserved, zero |

CRC32 is the reflected IEEE polynomial `0xedb88320`, initial and final XOR
`0xffffffff`. It detects accidental corruption; it is not authentication.
Both header and payload are written to a sibling `.tmp`, synced, closed and
renamed over the committed file. The target backend must guarantee atomic
same-directory replacement for this transaction to preserve the committed file
on failure. Migration also uses this transaction. Original raw files are never
deleted.
The single 1104-byte scratch buffer is shared by loads and flushes per MBA;
concurrent access returns `-EBUSY`. Mirror mutation remains locked separately
from filesystem I/O. Init/load/migration require a stopped game.

## EEPROM layout and portability

Bytes 0–15 are reserved for Arduboy/SDK settings; audio enable is byte 2.
Game data starts at byte 16. Erased audio (`0xff`) means enabled. Do not allocate
game data at byte 0 or persist a native `bool`, pointer or class as a format.
`EEPROM.get/put` copy native object representation and require trivially copyable
types. They do not normalize padding, endianness or target-dependent integer
widths (for example, AVR 16-bit versus a 32-bit host `int`). Use fixed-width
little-endian fields (`save_format::read16/read32`
and `write16/write32`) and migrate old layouts explicitly.

`examples/save_migration/migrations.hpp` demonstrates AVR u16/i16 fields at
byte 16 converted into explicit u32/i32 fields in schema 2, including signed
extension. The example uses its own `sdk_ab_save_20260913` ID, never a user's game
ID. Its schema 1 input must be provisioned explicitly to exercise migration.

## Hopper and Hollow save fixtures

`tests/fixtures/saves` contains read-only captured save-format fixtures,
with SHA256 and acquisition provenance. Both are 1024-byte raw files: audio
byte 0 is 1 and the remaining bytes are erased. Tests also construct nonempty
records using each pinned upstream game's checksum, and reject corruption.
EEPROM helper addresses and the record fields below use explicit
byte/word/dword operations.

| Game | Record offset | Signature | Record contents |
| --- | --- | --- | --- |
| Hopper | 800 | `0x024e424f` | u32 signature, 10 u16 scores, u16 play count, u32 play frames, u16 checksum |
| Hollow | 768 | `0x014e424f` | Same 32-byte layout |

The checksum is the sum of the first 15 little-endian words multiplied by
weights 1 through 15, modulo 65536. The migration examples accept an erased
record or the expected signature and checksum, preserve game bytes 16–1023,
reset the reserved area and explicitly move old audio byte 0 to byte 2.
They do not claim to recognize arbitrary third-party saves. Migration failures
leave both the prior envelope and raw source available for manual recovery.
