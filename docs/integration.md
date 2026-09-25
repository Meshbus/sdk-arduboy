# Integration boundaries

The SDK builds game code and its runtime into individual LLEXT applications.
The Meshbus firmware supplies Desktop lifecycle, ZUI input/display, indicator
audio, Zephyr timing/synchronization and filesystem services. Compatibility
headers belong on game include paths, not globally on firmware include paths.

| Name/path | Meaning |
| --- | --- |
| `sdk-arduboy` | GitHub repository |
| `MESHBUS_ARDUBOY_SDK_DIR` | Absolute root of this SDK, passed to game CMake |
| `LLEXT_EDK_INSTALL_DIR` in CMake | Extracted firmware-specific compiler/header/export inputs |
| `ZEPHYR_SDK_INSTALL_DIR` | Cross compiler/toolchain location |

`zephyr/module.yml` declares external CMake and Kconfig integration. A west-based
consumer must supply `ZEPHYR_ARDUBOY_CMAKE_DIR` and `ZEPHYR_ARDUBOY_KCONFIG`
adapter mappings. Adding this repository to a west manifest alone does not
provide those adapters or the required host APIs.

Standalone game builds include `meshbus_arduboy_llext.cmake` directly and do not
need west module discovery. Their CMake uses the EDK's `cmake.cflags`; strict
capability validation also needs `edk-release.json` with `exported-symbols`.
A stock Zephyr-only EDK without Meshbus services is insufficient.

Porting to another host requires implementations of the imported Desktop, ZUI,
indicator, timing, and filesystem interfaces, plus equivalent lifecycle and
save replacement semantics. `RuntimeHooks` customizes clocks and button mapping;
it does not replace the entire host backend. Check the actual package import
report against the target host. Do not enable features solely by defining their
compile-time macros.

See [capabilities](capabilities.md) for optional managed-stop and resource-location
exports. EDKs and host firmware have independent licenses; see
[third-party notices](../LICENSING.md) before distributing a combined app.

## Target selection

This SDK is consumed per target through its App EDK; it does not select one
default board. For each intended board:

1. Obtain an EDK from the exact board/qualifier and firmware configuration that
   will host the app. Check `edk-release.json` for target, host and export identity.
2. Select compatible compiler tools. The supplied example toolchain files select
   `arm-zephyr-eabi`; using another CPU architecture requires an appropriate
   toolchain adapter and validation, not just a different output directory name.
3. Build in a separate output directory. Check imports and memory limits using
   that EDK, even when another board built the same source successfully.
4. Validate the required host services and device behavior on that target. LLEXT,
   Desktop/ZUI, storage and any requested audio functionality depend on the
   firmware profile and hardware. Check each feature required by the game for
   the selected profile.

The 128×64 framebuffer and six button bits are the game's logical interface, not
requirements for a particular physical panel or button layout. The host supplies
display presentation and ZUI action mapping. Examples refer to logical A/B and
direction actions; the physical gesture for `ZUI_ACTION_CANCEL` (often long Back)
is defined by the target's input configuration. The runtime's default exit mask
selects that action. Audio errors/mute behavior follow the host adapter and
[the audio contract](audio.md).

The save backend must provide the documented filesystem operations and atomic
same-directory replacement semantics. Heap/stack limits, filesystem allocation
sizes and upload behavior must be checked per target. In particular, the
[sidecar installer](resources.md) has a fixed capacity estimate; this is
a tool limitation, not a universal board property.

Record the target and input identities with each validation result as described
in [testing](testing.md). Results apply to the tested EDK and configuration;
they do not qualify other targets.
