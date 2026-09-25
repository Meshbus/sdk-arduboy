# Troubleshooting

Record the SDK commit, `meshbus --version`, compiler version, EDK identity, exact
command and first failure. Keep output outside source directories or under
`build/`. Redact credentials, device identifiers and personal paths before
posting logs. Never treat a successful package build as device acceptance.

| Symptom | Check and response |
| --- | --- |
| Python, Clang, CMake or Ninja missing | Follow [prerequisites](getting-started.md). Host tests need Clang on `PATH`, including the Python diagnostic tests; `CXX` selects the C++ runner's compiler only |
| EDK lacks `edk-release.json` / `exported-symbols` | Obtain a current Meshbus App EDK from the intended host build. A stock Zephyr archive does not supply the full contract |
| Strict capability failure | Read the unavailable names in configure output. Use a supporting EDK or remove the feature requirement only if the game can work without it |
| External module mapping error | Use standalone EDK builds, or supply the consumer adapter mappings described in [integration](integration.md) |
| Undefined imports | Compare the build report with the actual host's exported symbols; rebuild against its matching EDK. Suppressing checks does not create missing host services |
| Loader heap/shared-stack rejection | Inspect the package report and reduce the app's requirement or choose a host with sufficient reservation. Build success alone does not measure stack high-water usage |
| Legacy GPIO/SPI/RGB compiler error | The API has no physical mapping. Port that behavior explicitly; use compatibility placeholders only after reviewing the gameplay consequences |
| Cannot open a resource after installation | Check the installed MBA/sidecar pair, resource-location capability and collection format. Fixed-path installation is different from managed atomic layout |
| Resource identity/version/CRC error | Rebuild and install the complete matching collection. Do not rename a different sidecar to satisfy the expected path |
| Save load/schema/migration failure | Preserve the existing `.sav` and `.dat`; inspect [save errors](saves.md) and the selected migration. A corrupt save must not silently become an empty one |
| Managed stop remains pending | Setup/loop must return or cooperate with `exit_requested()`. SDK delays cooperate, arbitrary busy loops do not. Do not overwrite an extension whose shutdown is unconfirmed |
| Two-file installer refuses existing files | It intentionally never replaces them. Use managed installation for registered apps; inspect incomplete uploads and remove only their exact paths when safe |
