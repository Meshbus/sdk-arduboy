# Isojourn FX example

Build `isojourn` through the [example index](../README.md), then install its MBA
and matching sidecar together. The [upstream lock](upstream.lock.json) records
source/resource hashes and licenses. The source game is unchanged.

Direction keys choose orientation; A advances one tile; long Back exits. Start
by moving south/east. The exploration prototype does not clamp world coordinates:
leaving the map triggers a bounded resource error and cooperative exit. This is
not a completed quest or proof of compatibility with all FX games.

See the [FX guide](../../docs/fx.md) for packing, supported APIs and memory bounds.

The runtime save ID is `isojourn` (`/extra/saves/isojourn.sav`).
