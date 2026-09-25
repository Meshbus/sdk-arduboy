# Capability contract

`MESHBUS_ARDUBOY_SDK_VERSION` identifies this SDK release;
`MESHBUS_ARDUBOY_SEMANTIC_REVISION` identifies its documented behavior contract.
Neither is a firmware commit, EDK digest, or host interface ABI. The legacy
`ARDUBOY_LIB_VER` selectors only choose upstream source branches. They do not
promise the complete upstream library.

`capabilities.json` is the machine-readable inventory, including implementation
bounds and known legacy degradations. Validation procedures and evidence
boundaries are described in [testing](testing.md). Resource bundles and FX implement the documented bounded subsets;
see [FX](fx.md). Unsupported operations remain explicit errors.

New targets use strict diagnostics by default. Calls to unmapped GPIO, SPI,
RGB, startup system buttons, volume, or legacy timing/settings placeholders
produce a compiler error naming the feature. Merely including an unused header
is supported. Ports can explicitly pass `COMPATIBILITY` to
`meshbus_arduboy_llext_add_app`; this emits warnings and preserves the listed
legacy behavior. Review each degradation for the game. Compatibility mode does
not grant hardware access or make placeholders correct.

Pass `REQUIRES clock input display graphics save audio` to declare requirements.
Runtime projects request these by default; the selected runtime must implement
every requested capability. Unavailable requirements fail configuration even in
compatibility mode. More specific bounds (such as unsupported repeating tone
scores) remain enforced by their API diagnostics.

Configure writes `arduboy-capabilities.json` with requested and actual features,
SDK and source Git revisions plus content digests, and EDK identity. Strict
projects require a Meshbus EDK containing `exported-symbols`.
The CLI reads the actual host export table, checks undefined non-weak imports,
and checks the configured heap and shared-stack limits before packaging. Its
`<id>.build.json` records these results. An EDK without an inventory emits
an explicit unchecked-import warning. Static checks cannot prove available
runtime heap, stack high-water usage, or device behavior.

From an activated Zephyr environment, run the repeatable host tests and
minimal Hello package build:

```sh
python3 tools/check.py --example hello --edk /path/to/llext-edk \
  --toolchain /path/to/zephyr-sdk --meshbus /path/to/meshbus \
  --output /path/to/task-evidence
```

This writes per-check logs and `results.json`; the package build report records
the input identities. It does not flash a device. Use `--example bounce` for the
pinned external game, subject to its [package capacity requirements](../examples/bounce/README.md).

## Optional host-dependent capabilities

The configurator reads the actual EDK export inventory. With `RUNTIME`,
`mbs_desktop_app_stop_requested` enables `managed_stop` and the
`MESHBUS_ARDUBOY_MANAGED_STOP` definition. The runtime checks the host request
through `exit_requested()` and returns through ordinary screen/audio/save cleanup.
It cannot forcibly interrupt arbitrary setup/loop code. Long computations should
check `exit_requested()`; SDK delays check it between bounded waits.

With `RUNTIME RESOURCE`, `mbs_desktop_app_resource_path` enables
`relocatable_resources` and `MESHBUS_ARDUBOY_RELOCATABLE_RESOURCES`. The resource
basename is resolved beside the executing MBA, allowing versioned managed
installation layouts. See [resources](resources.md).

Without those exports, the optional features are reported as unavailable.
SDK input handles exit, and resources use the compiled fixed path. To require
rather than merely detect them, add them to the
full requirements list, for example:

```cmake
REQUIRES clock input display graphics save audio managed_stop relocatable_resources
```

An explicit `REQUIRES` list replaces the default six runtime requirements;
resource/FX requirements are additionally inferred from their CMake options.
Do not define the feature macros manually or infer host support from SDK version.
