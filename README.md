# Meshbus Arduboy LLEXT SDK

Build Arduboy-style games as Meshbus `.mba` applications. The SDK provides a
bounded Arduino/Arduboy compatibility layer, a shared game runtime, and CMake
helpers for independent LLEXT builds against a matching Meshbus App EDK.

Build each game against the EDK exported for its target board and firmware
configuration. The SDK exposes a logical 128×64 monochrome framebuffer, six game
actions, EEPROM-style saves and bounded tone
playback; physical display, controls, audio and storage come from the host.
Host tests run without Zephyr or hardware. Device builds need the Meshbus CLI,
the target firmware's EDK and its compatible cross compiler; this repository
does not contain them. See [target selection](docs/integration.md#target-selection).

This is a specialized game-porting SDK, not a complete Arduino core, an AVR
emulator, or a drop-in implementation of every Arduboy library API. The upstream
version selectors do not certify full library compatibility. See the
[API support table](docs/api-compatibility.md) before porting a game.

## Start here

- [Getting started](docs/getting-started.md): prerequisites, host tests, and a
  first package using the original minimal game.
- [Minimal game](examples/hello/README.md): a small original example to copy.
- [All examples](examples/README.md): controls, expected results, and save IDs.
- [Troubleshooting](docs/troubleshooting.md): build, install, and runtime failures.

From this checkout's root, with Python 3 and Clang available:

```sh
python3 tests/run.py --build-dir /tmp/arduboy-host-tests
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 tools/check_publication.py
```

Use a fresh output directory for each validation run. These checks use controlled
adapters and sanitizers; they do not establish physical display, audio, or
power-loss behavior. See [testing and evidence](docs/testing.md).

## Documentation

| Task | Guide |
| --- | --- |
| Build independent sources or preprocess a Sketch | [Build integration](docs/building.md) |
| Understand Zephyr, Meshbus, and EDK boundaries | [Integration](docs/integration.md) |
| Select strict/compatibility mode and host features | [Capabilities](docs/capabilities.md) |
| Understand input, clocks, ownership, and framebuffer lifetime | [Runtime](docs/runtime.md) |
| Play tones and handle device audio settings | [Audio](docs/audio.md) |
| Define portable saves and migrations | [Saves](docs/saves.md) |
| Port old Arduboy scores and save formats | [Legacy adapters](docs/legacy.md) |
| Package a bounded read-only resource sidecar | [Resources](docs/resources.md) |
| Port the supported ArduboyFX subset | [FX](docs/fx.md) |
| Contribute or prepare a source release | [Contributing](CONTRIBUTING.md) |
| Inspect changes and version policy | [Changelog](CHANGELOG.md) |

## License and upstream code

Original SDK code is Copyright (c) 2026 FoBE Studio and licensed under
the [Apache License 2.0](LICENSE). Third-party code
and assets retain their respective licenses; see
[LICENSING.md](LICENSING.md). In particular, Arduboy2-derived
portions retain BSD notices and example toolchains declare Apache-2.0.
Third-party portions are not relicensed by the SDK default.

[UPSTREAM.md](UPSTREAM.md) records pinned sources, transformations, and
provenance limits. A compiled MBA also depends on its game, EDK, and host; the
SDK's Apache-2.0 license does not replace their license terms. Arduboy and
third-party project names identify compatibility and provenance, not endorsement.
