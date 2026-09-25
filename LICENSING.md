# Licensing

The root [LICENSE](LICENSE) applies to original SDK contributions. It does not
replace third-party copyright or license notices. Copyright for original SDK
contributions is held by FoBE Studio, under Apache-2.0. Third-party
attributions remain unchanged.

| Distributed portion | Attribution and terms | Source record |
| --- | --- | --- |
| `include/meshbus_arduboy/font5x7.hpp` | Arduboy2 font, with Arduboy/Adafruit ancestry; BSD-3-Clause and BSD-2-Clause | [SDK upstream lock](upstream.lock.json) |
| `include/Arduboy2.h`, `Arduboy2Base::drawCircle` | Adapted Arduboy2 rasterizer, including Adafruit-GFX ancestry; BSD-3-Clause and BSD-2-Clause. Original SDK portions use Apache-2.0 | [SDK upstream lock](upstream.lock.json) |
| `examples/bounce/upstream/` | Copyright (c) 2018 Zachariah Falgout (RetrobitCoder), MIT; original artwork credits are retained in the upstream README | [Bounce license](examples/bounce/upstream/LICENSE.txt), [lock](examples/bounce/upstream.lock.json) |
| `examples/isojourn/upstream/`, including `data.bin` | Copyright (c) 2021 pmwasson, MIT | [Isojourn license](examples/isojourn/upstream/LICENSE), [lock](examples/isojourn/upstream.lock.json) |
| `examples/{audio,bounce,isojourn,multitu,resource_fixture,save_migration,sketch_fixture,snapshot,timing}/toolchain.cmake` | Apache-2.0 as declared in each file; see the provenance limits in [UPSTREAM.md](UPSTREAM.md) | [Apache-2.0](LICENSES/Apache-2.0.txt) |

The [BSD-3-Clause](LICENSES/BSD-3-Clause.txt) and [BSD-2-Clause](LICENSES/BSD-2-Clause.txt) files
retain the upstream copyright notices, conditions, and disclaimers for
Arduboy2 and its Arduboy/Adafruit ancestry. They are extracted from the fixed
[upstream license bundle](https://github.com/MLXXXp/Arduboy2/blob/bc460a2cff1a3e116880991aa2f88bae4b2e3160/LICENSE.txt),
whose original file hash remains in [the source lock](upstream.lock.json).
The BSD section is reproduced with its original attributions, including the
SetSystemEEPROM example attribution; that example is not vendored here.

Upstream's separate LGPL sketches, zlib LodePNG, and other examples/tools are
not vendored here, so their license sections are not reproduced in `LICENSES/`. This
SDK's `examples/hello` is an original minimal game, not upstream's HelloWorld.

ArduboyFX is a pinned CC0 interface/format reference; its AVR library is not
linked or vendored. See its [fixed CC0 declaration](https://github.com/MrBlinky/ArduboyFX/blob/4d0270dd415ce459d3f79305a9733e9f73c05bf9/LICENSE)
and the [FX adapter guide](docs/fx.md). See [UPSTREAM.md](UPSTREAM.md) for the
provenance limits of compatibility code.

## Redistributing source and binaries

Keep this guide, the root Apache-2.0 license, applicable license texts, and original
copyright notices with source distributions. Binary distributions incorporating
BSD-derived code must reproduce its notices, conditions, and disclaimer in
accompanying documentation or other distribution materials. Bundled games keep
their own MIT notices. The current MBA builder does not automatically embed or
collect these texts: include them in your distribution alongside the package.

Toolchains, Meshbus, Zephyr, and exported EDK headers are separate dependencies.
Their licensing is determined by the versions and files actually distributed.
Review the actual EDK's `LICENSE.txt`, `NOTICE.txt`, and `LICENSES/` before
distributing combined applications. Apache-2.0 licensing of original SDK contributions is not a
claim that all compiled MBAs or host firmware can be distributed under Apache-2.0 alone.
