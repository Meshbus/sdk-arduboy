# Testing and evidence

## Host checks

Run from the SDK root with Python 3 and Clang (C++17, ASan/UBSan and pthreads):

```sh
python3 tests/run.py --build-dir /tmp/arduboy-host-tests
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 tools/check_publication.py
```

Choose a fresh build directory. `tests/run.py` builds each `tests/*.cpp` as a
separate executable with controlled host adapters. Its optional trailing names
select individual tests, for example `... --build-dir /tmp/arduboy-clock clock`.
`CXX` can override its compiler; the Python diagnostic tests explicitly require
`clang++` on `PATH`. `PYTHONDONTWRITEBYTECODE=1` keeps Python caches out of source.

| Check | What it demonstrates | What it does not demonstrate |
| --- | --- | --- |
| Sanitized C++ executables | Graphics, clocks, input, save transactions, resources/FX, audio and lifecycle behavior under test adapters | Actual host ABI, physical display/audio, flash endurance or power-cut recovery |
| Python unittest | Sketch generation, unsupported-API diagnostics, resource integrity and FX packer behavior | Cross compilation or installation on a device |
| Publication checker | Local document links, example prerequisites, declared license files, version consistency and vendored lock integrity | Remote source authenticity, exhaustive copyright provenance or secret scanning |
| CLI package build | C++ compilation/partial linking, host imports and declared package memory checks for the selected EDK | Loader execution, current free runtime memory, or device acceptance |

The GitHub [host-check workflow](../.github/workflows/host-checks.yml) runs on
Ubuntu and macOS without credentials, EDK downloads, devices or publication.
Its success must be observed after pushing; a locally checked workflow is not a
completed GitHub Actions run.

## Repeatable package check

With the inputs described in [Getting started](getting-started.md):

```sh
python3 tools/check.py --example hello --edk "$LLEXT_EDK_INSTALL_DIR" \
  --toolchain "$ZEPHYR_SDK_INSTALL_DIR" --meshbus "$MESHBUS_CLI" \
  --output /absolute/path/to/task-evidence
```

This runs the C++ and Python checks and packages Hello, producing per-check logs
and `results.json`. Publication checks are a separate command above. The EDK can
be an archive or extracted directory. See `packages/hello.build.json` for the
package's actual inputs, exports and memory checks. Use the [example index](../examples/README.md)
to build other fixtures. No command in this section contacts a device. Without
`--example`, the tool builds Bounce; check its
[package capacity requirements](../examples/bounce/README.md).

## Recording host and package results

Record the SDK revision or source digest, host OS, compiler and Python versions,
exact commands, results, and log location. Package results also need the CLI
version, matching EDK identity, target board and firmware configuration, toolchain
version, package hash, and build report. Keep these records with the tested
artifacts; a result describes only those inputs.

## Recording device acceptance

Record firmware identity, matching EDK, SDK/game revisions, target board, package
hash, command/log location, observed result and observer/date. Check load/start,
controls and exit, the fixture's expected frames, audible behavior where relevant,
and save/reload without touching unrelated saves. Keep static/build, software
framebuffer capture, physical observation, and power-cut testing separate.
Maintain separate acceptance records for each board and firmware configuration.
A result on one target is not evidence for another target's exports, memory,
peripherals or storage behavior. Only mark a capability device-qualified for
that target when its record is available. Managed stop and relocated resources
also require matching host exports; the default
host stubs do not cover those enabled host-dependent branches.
