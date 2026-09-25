# Changelog

## 0.2.0-dev.1

The initial SDK provides:

- Bounded Arduino, Arduboy2, and legacy Arduboy compatibility interfaces.
- A shared Sketch runtime for input, frame timing, display, audio, and saves.
- Versioned save envelopes, explicit schema migration, and read-only resources
  with a bounded ArduboyFX adapter.
- Independent C++ translation units and Sketch preprocessing for LLEXT builds
  against a matching host EDK.
- Optional managed stop and resource location selected from host exports.
- Examples, host tests, and offline source-publication checks.

`0.2.0-dev.1` is the source development identifier. See
[capabilities](docs/capabilities.md) for supported interfaces and limits,
[testing](docs/testing.md) for validation procedures, and
[licensing](LICENSING.md) for original and third-party terms.
