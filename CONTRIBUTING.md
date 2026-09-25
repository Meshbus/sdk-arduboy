# Contributing

Start with [Getting started](docs/getting-started.md), the [API contract](docs/api-compatibility.md),
and [testing](docs/testing.md). Report bugs through this repository's GitHub
Issues. Include SDK/CLI/EDK identities, the smallest reproducer,
expected/actual behavior and whether the failure is host, build, or device-only.
Remove credentials and personal device data from attachments.

## Changes

Keep upstream game snapshots unchanged and put adaptations in the surrounding
bridge. For an intentional import update, follow [UPSTREAM.md](UPSTREAM.md),
retain copyright notices, refresh the pin/hashes, and check the affected license.
Original contributions are accepted under this project's Apache-2.0 license;
contribute only material you have the right to license. Third-party material keeps its own
terms and belongs in [LICENSING.md](LICENSING.md).

Use narrow commits with an imperative `area: summary` title below 72 characters
and a body explaining the problem, approach and actual validation. Document
unperformed device checks explicitly. Do not add a DCO sign-off on another
person's behalf. No CLA or additional sign-off requirement is introduced here.

Run the three host checks in [testing](docs/testing.md). Changes to CMake,
examples or imported headers also need a package build against a named EDK.
Update user-facing documentation and add a changelog entry for any
observable behavior or integration change. Do not relabel an unknown API as
supported because one particular game compiles.

## Source release preparation

1. Run host/publication checks and relevant EDK builds; record exact input
   identities, results, and limits with the release artifacts.
2. Review upstream notices, contributor rights, binary asset origins and source
   package contents. Preserve all required license files; an Apache-2.0 root license
   alone does not cover bundled third-party code or an EDK.
3. Review the source distribution for credentials, private URLs, personal data
   and accidentally tracked outputs. Publication checks do not scan for secrets.
4. Check that the public quick start's dependency links are accessible, or clearly
   state which artifacts still need to be supplied by a firmware provider.
5. Choose a release version, synchronize `capabilities.json` and
   `include/meshbus_arduboy/capabilities.hpp`, and write a dated changelog entry.
   Create a tag and publish only as part of the maintainer's release action.
6. Inspect the resulting source archive for notices, lock files and all example
   resources. A release must state what was tested on hardware separately.

## Version policy

`0.2.0-dev.1` is the current development identifier.
The SDK version identifies source distribution. `semantic_revision` identifies
the documented behavior contract; increment it for incompatible contract changes.
Upstream selectors, EDK identity, host ABI, firmware version, and game version
are independent. Rebuild against a compatible host EDK after host interface
changes; matching SDK versions alone do not guarantee MBA binary compatibility.
