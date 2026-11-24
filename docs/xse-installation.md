# XSE executables
An XSE is a native executable, not an archive. MyOs launches it directly using the host process API. No registry is required.

- Type `calc.xse` in its directory.
- Type `run calc` to search the current directory and then C:/Apps.
- Explicit virtual paths work with run.
- The application starts in its executable directory.

Local installation currently accepts a trusted single C++17 source in C:/Downloads/<id>.cpp. It copies the source into a unique staging directory, compiles to app.xse, publishes without replacing an existing executable, and removes the staging directory. Original local input is retained. Store-owned downloads must be cleaned by the Store integration after successful placement.

Linux uses c++; Windows currently uses g++ and the native process API. A host compiler and hard-link-capable filesystem are required. Native programs execute with host account permissions; the virtual drive is not a process sandbox.

Remote Store retrieval, multi-file builds, updates and interrupted-install recovery are not implemented by this local installer. Regression tests cover compilation, launch, lookup, relocation, duplicate rejection and failed-build cleanup.
