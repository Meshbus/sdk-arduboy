# Independent C++ translation units

Use `meshbus_arduboy_llext_add_app(game RUNTIME SOURCE bridge.cpp)` for one game
source, or `RUNTIME SOURCES main.cpp physics.cpp assets.cpp` for independent
sources. SOURCE and SOURCES are mutually exclusive. RUNTIME defines
MESHBUS_ARDUBOY_RUNTIME and adds runtime.cpp and eeprom_runtime.cpp exactly once;
applications include runtime.hpp, never runtime.cpp. EEPROM bridge definitions
and save scratch storage live in the implementation object shared by all TUs.

SOURCE bridges without RUNTIME can define
MESHBUS_ARDUBOY_LLEXT_GAME_ENABLE_EEPROM to include the EEPROM implementation object.
SOURCE without RUNTIME uses -fno-weak; RUNTIME and SOURCES use real COMDAT/weak linkage
so inline function statics are shared and local/anonymous symbols remain private.
Exceptions, RTTI, thread-safe static initialization and __cxa_atexit remain
unsupported. Global constructors and .fini_array destructors run through the host
LLEXT lifecycle. Constructors must not call game runtime APIs before run_sketch.

Each source compiles to a separate ARM object with a compiler depfile. The
cross compiler identifies its matching linker, which combines objects with
`ld -r --force-group-allocation` into ET_REL. A relocatable-only script merges
COMDAT members into .text/.rodata/.data and a single .bss after symbol
resolution. Zephyr LLEXT rejects multiple SHT_NOBITS allocations, so leaving
per-inline-static .bss sections unmerged is not a usable package. This intentionally bypasses executable-only picolibc specs
and does not link host libraries into the extension. The Meshbus CLI packages
the resulting .llext normally; required imports must exist in the actual host.
Compiler flags come from that host's EDK and the CLI's compiler wrappers.

`examples/multitu` includes EEPROM interfaces in two separate sources and tests
same-name static/anonymous variables, a shared inline-function static, strong
versus weak resolution, and global construction/destruction. Its source files
are never textually included into one another. `examples/bounce` uses a
single game-source bridge with the common runtime linked separately.

Record target-board acceptance as described in [testing](testing.md): a successful ARM
partial link alone does not prove runtime relocations, constructor execution or cleanup.

## Sketch generation

Add `SKETCH path/to/Game.ino` alongside RUNTIME and the app bridge SOURCE. The
explicit main file comes first, then other .ino/.pde files in its directory in
alphabetical order. Ordinary .cpp files in that directory compile independently.
The source checkout is never rewritten; generated Sketch code lives under the
binary directory. A bridge includes meshbus_arduboy/sketch.hpp for the standard
setup/loop entry points; it does not declare internal Sketch functions.

This follows Arduino's [documented preprocessing order, Arduino.h insertion,
prototype generation and #line behavior](https://docs.arduino.cc/arduino-cli/sketch-build-process/).
The explicit main-file argument also supports upstream checkouts whose containing
directory has been renamed. Headers are not implicitly included and no unknown
libraries are downloaded.

The generator tokenizes comments, ordinary/raw strings, directives and balanced
scopes. Supported automatic prototypes are ordinary top-level functions with
identifier-based types, pointer/reference parameters, and optional static/inline
qualifiers. Existing matching declarations are preserved. Types needed by a
prototype must already be declared before the first function definition. Calls
in earlier global initializers need explicit declarations, just as ordinary C++.
Templates, namespaces/linkage blocks, function-pointer declarators, attributes,
array/default parameters and conditionally defined functions are diagnosed as
unsupported; put those functions in a .cpp and declare them in a header. This
bounded generator is not a general C++ parser and does not guess macro-expanded
function signatures. Compiler diagnostics retain original Sketch file/line paths.

Compiler depfiles track nested includes and EDK-generated configuration headers;
cmake.cflags is also an explicit input. Resource generation must declare its own
inputs/outputs using normal CMake rules. For example configure_file inputs trigger
reconfiguration and their generated headers are tracked by consumers' depfiles.
For a custom generated resource needed before the first compile, supply its
output in DEPENDS and declare the generating command. No recursive list of
all upstream headers is required. Unchanged input leaves objects untouched.
