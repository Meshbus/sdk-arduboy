# Examples

Follow [Getting started](../docs/getting-started.md) to set `MESHBUS_CLI`,
`LLEXT_EDK_INSTALL_DIR`, `ZEPHYR_SDK_INSTALL_DIR`, `MESHBUS_ARDUBOY_SDK_DIR`, and
`TARGET_LABEL`. Select the EDK for the intended board and firmware configuration.
From the SDK root, select one directory name below:

```sh
example=hello
"$MESHBUS_CLI" llext --llext-sdk "$LLEXT_EDK_INSTALL_DIR" \
  --zephyr-sdk "$ZEPHYR_SDK_INSTALL_DIR" \
  -o "$PWD/build/$TARGET_LABEL/$example" "$PWD/examples/$example" -- \
  -DMESHBUS_ARDUBOY_SDK_DIR="$MESHBUS_ARDUBOY_SDK_DIR"
```

Successful builds produce `<example>.mba` and `<example>.build.json` in that
output directory. Resource examples additionally produce a sidecar/install
collection. Installation and physical acceptance are separate steps.

| Example | Purpose and expected observation | Controls |
| --- | --- | --- |
| [hello](hello/README.md) | Original minimal game: title and movable square | Left/right move, A centers |
| [bounce](bounce/README.md) | Pinned MIT game; menu, gameplay and score | A starts/serves, arrows move, B menu |
| [isojourn](isojourn/README.md) | Pinned exploration prototype and read-only FX | Arrows orient, A advances; avoid map bounds |
| [audio](audio/README.md) | Three-note playback and persistent mute preference | A plays, B toggles/saves, Up stops |
| [multitu](multitu/README.md) | Independent objects, weak symbols and lifecycle; `Multi TU PASS` | Exit after observing screen/logs |
| [sketch_fixture](sketch_fixture/README.md) | Preprocessing/dependencies; `order PASS`, `17 / 7` | Exit after observing screen |
| [snapshot](snapshot/README.md) | Stable submitted frame despite source mutations | A advances three controlled patterns |
| [timing](timing/README.md) | 15/30/60 FPS intervals and cancellable delay | Long Back cancels final wait |
| [save_migration](save_migration/README.md) | Explicit 16-bit-to-32-bit save layout migration and reload | A increments; provision schema 1 to exercise migration |
| [resource_fixture](resource_fixture/README.md) | Bounded sidecar reads, two frames and level data | A switches frame |

All examples use CANCEL to exit. Controls above describe logical actions;
"long Back" is shorthand for the CANCEL gesture on hosts with that mapping.
Use the physical mapping provided by the target firmware. See
[target selection](../docs/integration.md#target-selection). Fixture-specific
save IDs are documented in each README. Inspect
source and preserve existing saves before provisioning migration data.

These expected observations are acceptance instructions, not a statement that
all examples passed on every firmware. See [testing](../docs/testing.md).
