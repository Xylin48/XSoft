# XSE installation lifecycle

XSoft generates XSE packages locally from downloaded Store branch contents. The Store does not need to distribute prebuilt XSE files. MyOs launches installed packages through the XSE runtime.

## Transaction

1. Download into a unique installer-owned staging directory.
2. Validate manifest, package identity, paths and resources. Reject paths escaping staging and unsupported package formats.
3. Prepare application files and dependencies inside staging.
4. Generate the XSE locally, after preparation and before permanent placement.
5. Verify the generated package, then place it in the virtual C: drive using a same-filesystem rename where possible.
6. Verify permanent placement and commit the application registry entry. Retain the previous installed version until the transaction succeeds.
7. Remove installer-owned temporary downloads, extracted files and intermediate build artifacts. Never remove permanent resources or user data.

On failure, roll back incomplete placement and registry changes, preserve the previous installed version, and clean only the staging directory owned by this transaction. Cleanup failures must be reported and retried without deleting unrelated paths. Do not follow symlinks during cleanup. An interrupted installation must be recoverable.

## Implementation boundaries

The package generator, package manager, and runtime belong to XSoft; Store retrieval is a separate input provider. The package binary layout, manifest schema, execution ABI and permanent directory layout remain to be finalized against existing PyOs behavior. Do not treat an arbitrary downloaded native executable as a validated XSE.

## Acceptance tests

- Successful install generates XSE before placement and leaves no temporary files.
- Failed generation or verification leaves no installed partial package.
- Failed update preserves the prior installed package and user data.
- Registry failure rolls back placement.
- Cleanup never removes unrelated files, permanent resources, or user data.
- Interrupted installation can be recovered on next startup.
- Local installation works without Store access once input files are available.
- Windows and Linux builds exercise the same lifecycle.
