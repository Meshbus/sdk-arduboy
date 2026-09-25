# Sketch preprocessing and dependency fixture

`sketch/sketch.ino` is the main file. `a.ino` and `z.ino` follow alphabetically;
their dynamic initializers must produce order=123. setup calls drawLater without
a hand-written prototype. worker.cpp remains a separate compilation unit and
reads a nested header plus a resource header generated in the build directory.
The stock display shows order PASS and 17 / 7. Long Back exits.

Changing detail/value.hpp must rebuild worker.cpp only. Changing resource.in
regenerates generated_resource.hpp and rebuilds its consumer. An unchanged build
must not compile objects. Test changes should use a source copy to keep the
stock fixture and third-party checkouts intact.

The runtime save ID is `sketch_fixture` (`/extra/saves/sketch_fixture.sav`).

Build commands and package outputs are in the [example index](../README.md).
