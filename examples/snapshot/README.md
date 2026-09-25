# Display snapshot acceptance fixture

Initially publish 1024 bytes of 0x55 using display(true), then overwrite the
cleared source with 0xaa. A/Select advances to 0x0f/0xf0 using display(true).
A/Select again publishes 0x33 using display(false), overwrites the source with
0xcc, then stops publishing. Each stage remains until explicitly advanced, so
serial capture latency cannot cross an automatic transition. Target framebuffer dumps
must exactly equal each published pattern. Long Back exits. No snapshot
allocation occurs after MBA loading.

The runtime save ID is `snapshot` (`/extra/saves/snapshot.sav`).

Build commands and package outputs are in the [example index](../README.md).
