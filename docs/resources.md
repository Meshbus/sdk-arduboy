# Read-only resource sidecars

Opt-in `RESOURCE`, `RESOURCE_VERSION`, and `RESOURCE_DIRECTORY` arguments to
`meshbus_arduboy_llext_add_app` generate a MARB v1 sidecar, its manifest, and
`arduboy-resource.hpp` in the build directory. The target name must equal the
MBA id. Use a stable resource version and a canonical directory below
`/extra/apps/`; each path component uses letters, digits, underscores or dashes.
Set `SketchConfig::resources = &arduboy_resource` in the app bridge. Resources
require the common runtime and an EDK exporting `fs_seek`.

Before calling game setup or acquiring a screen, the runtime opens the sidecar
read-only and verifies its identity, version, exact length, and full payload
CRC32 against the compiled resource contract. The 64-byte little-endian header
contains magic `MARB`, format u16=1 at 4, header size u16=64 at 6, resource
version u32 at 8, payload length u32 at 12, payload CRC32 at 16, reserved zero
u32 at 20, NUL-padded identity[32] at 24, reserved zero u32 at 56, and CRC32 of
the first 60 bytes at 60. CRC32 uses the IEEE polynomial. These are accidental
corruption and pairing checks, not publisher authentication.

`resource_read(offset, destination, size)` addresses the payload (header
excluded). Only consume output after a zero return; an I/O error can leave a
partial copy. Zero-length reads at EOF are allowed; offsets and lengths beyond
EOF return `-ERANGE`. Reads use one 256-byte cache within the existing game
thread's AppContext, with no payload-sized heap allocation. The same cache
scans startup data. Errors distinguish missing files (`-ENOENT`), short files
(`-ENODATA`), wrong versions (`-EPROTONOSUPPORT`), wrong identity (`-EXDEV`), and
corrupt data (`-EBADMSG`). All startup failures and normal exits close the file.
The caller must keep the resource spec alive throughout `run_sketch` and must
not change the backing file while the game runs. No resource writes or FX API
compatibility are implied.

The CLI additionally produces `<id>.install/` with the MBA, sidecar, and
`install.json`, whose SHA-256 entries bind the exact local installation files.
Install a new collection using the activated development environment with
`smpclient` available. Set `DEVICE_PORT` to the selected device's serial port:

```sh
python tools/install.py /path/to/id.install --meshbus /path/to/meshbus \
  --port "$DEVICE_PORT" --report /path/to/install-result.json
```

`--dry-run` checks local hashes, device capacity, and absence of destination
files without writing. Installation uses 256-byte SMP chunks, uploads the
sidecar first and MBA last, and reads both back. It refuses to replace existing
files. The script estimates capacity by rounding each file to 4 KiB and
adding two metadata blocks. This value is fixed in the tool, not discovered from
the target filesystem. Confirm that estimate is suitable for the target backend;
use the matching host installer when it is not. Even a suitable estimate cannot
guarantee allocation if another writer changes the filesystem. Arrange exclusive
installation access.
Interrupted uploads may leave new incomplete files; inspect and remove only
those paths before retrying. Builds without `RESOURCE` produce a single MBA.

`examples/resource_fixture` generates two graphic frames and level data in the
build directory. A switches frames, and long Back exits. Its startup performs
a read across the 256-byte cache boundary. See [testing](testing.md) for device
acceptance; native tests alone do not establish device resource performance.

## Resource locations and installation formats

An EDK exporting `mbs_desktop_app_resource_path` enables
`relocatable_resources`. The runtime passes the sidecar basename to that host
resolver and opens the path beside the executing MBA. Resolver errors are
returned without falling back to a different resource version. Without the
capability, it opens `ResourceSpec.path`, the compiled fixed path. Require the
capability explicitly when your deployment depends on a managed version directory.

`tools/install.py` is the two-file `install.json` installer described above.
It does not implement the managed package registry, replacement transactions or
atomic version selection. CLI-managed projects use their generated
`package.json` collection and the corresponding `meshbus app` installer. Do not
feed one format to the other. The script's `--dry-run` is read-only on
the device but still opens a connection and queries capacity and destinations.
