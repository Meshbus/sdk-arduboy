# Upstream provenance

Keep vendored game files unchanged. Put lifecycle, CMake, and platform adaptations
outside `upstream/`. Lock records identify the reviewed inputs, not permission to
silently update them. Source changes require a new pin, license review, file
hashes, and affected tests.

| Component | Fixed source | Local transformation |
| --- | --- | --- |
| Arduboy2 | [bc460a2](https://github.com/MLXXXp/Arduboy2/tree/bc460a2cff1a3e116880991aa2f88bae4b2e3160) | `font5x7` array bytes retained under an SDK declaration; `drawCircle` body adapted into the compatibility class |
| Bounce | [2f62ab5](https://github.com/RetrobitCoder/Bounce/tree/2f62ab597d6decc659409ccf83274cf0348994d1) | Tracked files in `examples/bounce/upstream` unchanged; bridge and generated Sketch separate |
| Isojourn | [1cfc136](https://github.com/pmwasson/isojourn/tree/1cfc136cd03a82561983da989386192821018d4f) | Six locked source/resource/license files unchanged; deterministic FX packing in build output |
| ArduboyFX | [4d0270d](https://github.com/MrBlinky/ArduboyFX/tree/4d0270dd415ce459d3f79305a9733e9f73c05bf9) | API/format reference; no upstream AVR library linked |

The [root lock](upstream.lock.json) records Arduboy2 reference-file hashes and
the extracted font hash. It also records the local BSD/Apache license-file
hashes for the files in `LICENSES/`; the original upstream license file hash
identifies the remote reference, not a separately distributed local snapshot.
Game locks are adjacent to their vendored directories.
`tools/check_publication.py` checks local lock integrity offline. It does not
independently prove that a newly written lock matches a remote repository.

The vendored Bounce README uses root-relative `/Images/...` links. Those images
are not included here. Read its [original README](https://github.com/RetrobitCoder/Bounce/blob/2f62ab597d6decc659409ccf83274cf0348994d1/README.md)
for the images and artwork credits; the local source snapshot stays unchanged.
Isojourn provides a binary resource, not an asset-source generator. The local
packer reproduces compressed bytes from that pinned binary, not the original art.

## Compatibility implementation and provenance limits

`Arduino.h`, `Print.h`, `ArduboyTones.h`, `Arduboy.h`, `compat.hpp`, and the
runtime are SDK compatibility implementations carrying Apache-2.0 declarations.
They route supported operations into Meshbus and do not link an Arduino core.
File names and API similarity alone do not establish copied implementation.
The source records do not provide a line-by-line independent-authorship
attestation for all compatibility code. Maintainers should preserve import and
design records and investigate additional copied portions before claiming an
exhaustive copyright audit. Confirmed Arduboy2 portions are listed separately
in [third-party notices](LICENSING.md).

The nine toolchain files identified in [LICENSING.md](LICENSING.md) declare
Apache-2.0. No separate upstream URL or copyright owner is documented for those
files. Their declarations and the full license text are included; no additional
copyright ownership is inferred for those files.

The Hopper/Hollow save fixtures contain one enabled-audio byte followed by erased
bytes; their paths and hashes are in `tests/fixtures/saves/provenance.json`.
They contain no recorded game progress. They are compatibility fixtures, not
redistributions of those games. Their upstream game sources are outside this
checkout; legacy schema support is limited to the documented layouts.
