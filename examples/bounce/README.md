# Bounce compatibility example

Unmodified game: https://github.com/RetrobitCoder/Bounce
Pinned commit: `2f62ab597d6decc659409ccf83274cf0348994d1` (MIT; upstream license retained).
Build this directory using the Meshbus CLI and the EDK matching the target
board and firmware configuration.
The bridge uses the installed SDK headers; it does not carry a private SDK copy.
A/Select starts or serves, arrows move, B/Back navigates the menu, long Back exits.
The example uses `RUNTIME`: delays are cancellable and audio uses owned host
playback. See [runtime](../../docs/runtime.md) and [audio](../../docs/audio.md).
Build commands and expected outputs are in the [example index](../README.md).
The [lock](upstream.lock.json) records the unchanged vendored files; see
[upstream provenance](../../UPSTREAM.md) for the upstream README's omitted images.

## Package capacity requirements

Inspect the package build report for the selected EDK. Bounce must fit the host's
per-app heap and reservation limits. If packaging rejects its memory request,
use a host/EDK with sufficient capacity or reduce the application's requirements.
Do not bypass the check or treat an intermediate `.llext` as an accepted package.
Use `hello` for a minimal first build; device execution requires separate
validation as described in [testing](../../docs/testing.md).

The runtime save ID is `bounce` (`/extra/saves/bounce.sav`).
