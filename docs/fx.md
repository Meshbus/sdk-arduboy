# Read-only FX compatibility

Enable `RUNTIME FX RESOURCE ...` in `meshbus_arduboy_llext_add_app`.
The resource collection and installation contract remains [MARB](resources.md).
`tools/fx_pack.py` turns the original logical FX data into independently
compressed 256-byte blocks. `FX::begin(page)` verifies the development page and
index. Reads never address physical SPI or mutate the original data.

Supported calls are `begin`, `readDataArray`, `drawBitmap` with `dbmNormal` or
`dbmMasked`, and `display`. Bitmap dimensions are big-endian, vertical pixels
are LSB first, and masked bytes alternate image/mask. Dimensions must be
1..128; frames use an 8-bit index. OLED enable/disable only arbitrate a bus on
AVR, so they have no action with this software framebuffer. Other FX APIs are
absent; invalid modes/data/bounds log an error and request cooperative exit.
FX writes, save regions, flash carts, grayscale, audio streaming and alternate
bitmap modes are unsupported. Sketches must return from loop to honor exit.

The decoder owns one 256-byte cache, in addition to the resource reader's
256-byte cache; drawing uses at most 256 stack bytes for a visible page row.
No full resource allocation or dynamic decompression allocation is made.

## FXPK version 1

All numbers are little-endian unless noted. Header: `FXPK`, u16 version 1,
u16 block size 256, u32 logical length (1..16 MiB), u16 development page,
u16 zero, u32 block count. Follow with block-count+1 u32 absolute offsets.
First offset equals header plus table length; offsets strictly increase;
last offset equals the physical payload size. Each encoded block is at most
512 bytes. Every decoded block is 256 bytes except the logical tail.
A token below 128 introduces token+1 literal bytes. Otherwise it introduces
(token & 127)+3 copied bytes and one distance byte (zero means 256).
References stay inside the decoded block and may overlap. Both input and
output must end exactly at their declared boundaries. MARB CRC and install
SHA bind the compressed bytes; the packer checks exact raw-byte round trips.

## Isojourn example

`examples/isojourn/upstream.lock.json` pins the MIT game, source/resource hashes,
and the CC0 FX reference. Vendored upstream files are unchanged. The original
82,224-byte binary becomes 27,271 compressed bytes, plus the 64-byte MARB header.
The bridge adds lifecycle and position diagnostics. Upstream supplies a binary
resource, not a source asset generator; regeneration here means deterministic
lossless packing of that pinned binary.

Isojourn is an exploration prototype. Direction keys select orientation and
A advances one tile. The upstream code does not clamp world coordinates;
walking outside its map triggers a bounded resource error and exits. Start
by moving south/east. This does not establish compatibility with every FX
game or a completed quest. See [testing](testing.md) for evidence boundaries and [the example](../examples/isojourn/README.md) for controls.
