# Getting started

## Choose a workflow

Host tests need only this checkout, Python 3 and Clang with C++17, ASan, UBSan,
and thread support. They use the stubs in `tests/stubs`; no Zephyr installation,
firmware checkout, EDK, or device is required. Windows host testing is not
qualified; use a Linux environment if the required POSIX tooling is unavailable.
See [testing](testing.md) for commands and coverage.

To build an `.mba` for a selected Meshbus target, additionally obtain:

| Input | Requirement and source |
| --- | --- |
| Meshbus CLI | A compatible `meshbus` executable; see the [Meshbus repository](https://github.com/Meshbus/meshbus) and its `DISTRIBUTION.md` |
| App EDK | Exported from the intended board and LLEXT-enabled Meshbus firmware configuration; keep its license files and identity manifest |
| Cross compiler | Zephyr SDK matching that EDK, with `arm-zephyr-eabi` tools; [Zephyr SDK releases](https://github.com/zephyrproject-rtos/sdk-ng/releases) |
| Build tools | CMake 3.20 or newer, Ninja, Python 3, and Git for source identity |
| Compiler helpers | Let the Meshbus CLI supply its compiler and `xxd -ip` wrappers through `PATH` |

The examples require C++17; unsupported C++ runtime features are listed in
[building](building.md). Record the tools and input identities used for each
validation run as described in [testing](testing.md).

Obtain a matching CLI and target EDK from your firmware provider before package
builds. Host tests can run using this checkout without those inputs.

## Obtain the source and run host checks

Clone [sdk-arduboy](https://github.com/Meshbus/sdk-arduboy), then open a terminal
in the SDK root and run:

```sh
python3 tests/run.py --build-dir /tmp/arduboy-host-tests
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 tools/check_publication.py
```

Use a fresh output directory for a new run. Expected results are `PASS` for each
C++ executable, `OK` from unittest, and a publication-check success message.
Compatibility warnings and deliberately injected error logs are expected in
negative tests; process failures are not. No device is contacted.

## Build the minimal game from this repository

Select the board/firmware EDK first. `TARGET_LABEL` below only names the local
output directory; the EDK supplies the actual target. Use a fresh output directory
and the matching EDK/toolchain for each board or firmware configuration. The same
source can be rebuilt for multiple supported targets; an MBA is not a universal
binary. See [target selection](integration.md#target-selection).

Run from the SDK root. Replace the three input paths with absolute paths to
existing files/directories; no environment activation is needed when these
tools and Python are already on `PATH`.

```sh
export MESHBUS_ARDUBOY_SDK_DIR="$PWD"
export TARGET_LABEL=target-board
export MESHBUS_CLI=/absolute/path/to/meshbus
export LLEXT_EDK_INSTALL_DIR=/absolute/path/to/target-edk.tar.xz
export ZEPHYR_SDK_INSTALL_DIR=/absolute/path/to/zephyr-sdk

"$MESHBUS_CLI" llext \
  --llext-sdk "$LLEXT_EDK_INSTALL_DIR" \
  --zephyr-sdk "$ZEPHYR_SDK_INSTALL_DIR" \
  -o "$PWD/build/$TARGET_LABEL/hello" "$PWD/examples/hello" -- \
  -DMESHBUS_ARDUBOY_SDK_DIR="$MESHBUS_ARDUBOY_SDK_DIR"
```

The CLI accepts an EDK archive or an extracted directory. Its configure step
supplies the extracted EDK path to CMake. Keep the compiler wrappers selected
through `PATH`; selecting the bare cross compiler bypasses package flags.
Expect `build/$TARGET_LABEL/hello/hello.mba` and `hello.build.json`. Inspect the report's
import and memory checks before attempting installation. An exit code of zero
proves package construction for that EDK, not execution on a device.

For a vendored game, use `examples/bounce` with a separate output directory,
and check its [package capacity requirements](../examples/bounce/README.md).
For other examples see the [index](../examples/README.md).

## Next: install and run

Build commands above are device-free. Installation is a separate operation.
Use the CLI/firmware provider's matching [app development guide](https://github.com/Meshbus/meshbus/blob/main/docs/app-development.md)
for managed projects, device binding, installation, start/stop, and recovery.
A managed project can be created with `meshbus app new hello --template arduboy`
on CLI versions supporting that template; it is distinct from copying this
repository's low-level `llext` example.

Resource examples produce an MBA/sidecar collection; use the matching managed
installer, or the [two-file resource installer](resources.md), keeping the two
formats separate. The latter additionally requires `smpclient` from the
firmware provider's Python environment. Its `--dry-run` still contacts a device.
