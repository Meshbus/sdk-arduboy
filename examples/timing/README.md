# Timing acceptance fixture

Submit a frame, wait 3000 ms, measure 30 intervals at 15/30/60 FPS, then wait
60000 ms. Long Back must cancel that final wait and return to Launcher.
The pinned Arduboy2 clock uses integer millisecond periods: expected spans are
1980, 990, and 480 ms before scheduler overhead. Serial output reports actual
measurements. This fixture does not rely on a fixed external ZUI tick.

The runtime save ID is `timing` (`/extra/saves/timing.sav`).

Build commands and package outputs are in the [example index](../README.md).
