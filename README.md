# XSoft / MyOs

XSoft hosts **MyOs**, the C++ successor to PyOs: a lightweight terminal-based application and game hub. This is a work-in-progress port, not a PC emulator or graphical desktop.

## Build

Requires a C++17 compiler, CMake 3.12+, and OpenSSL Crypto development libraries.

```sh
cmake -S . -B build
cmake --build build
./build/xsoft
```

On Windows, run the executable produced by the selected CMake generator (for example `build/Debug/xsoft.exe`).

## XSoft virtual C: drive

XSoft creates a **real folder-backed drive** beside the XSoft executable (regardless of your terminal's working directory):

```text
./xsoft/
  C/
    MyOs/
      System/
        Accounts/
          users.db      (created after registration)
    Users/
    Program Files/
    Temp/
```

The MyOs terminal displays `C:\\>` and uses MyOs-style paths. `pwd` displays the virtual path, `cd` navigates within C:, and `ls`, `dir`, `mkdir`, `mkfile`, `touch`, `rmdir`, `rmfile`, and `cat` operate on the folder-backed drive. `reboot` resets the virtual working directory to `C:\\` and returns to login without rebooting the host.

For example: `cd Users`, `cd ..`, or `cd C:\\Program Files`. A drive letter other than C: is rejected. Paths attempting to leave C: are rejected; symlinks in user-supplied paths are rejected. This is a **basic path boundary**, not a security boundary against malicious local processes or filesystem race attacks. Do not treat it as a hardened sandbox.

**Portability note:** The storage location is anchored to the **executable directory**, so launching from another working directory uses the same drive. When using a CMake build, this is beside the built executable (often inside `build/`), not necessarily beside the source checkout. Move the executable and its `xsoft/` directory together to retain its data.

## Standalone accounts

At startup choose `register`, `login`, or `quit`. Passwords are salted using cryptographic randomness and hashed with PBKDF2-HMAC-SHA256 (310,000 iterations), not saved as plaintext. The account database now resides in `xsoft/C/MyOs/System/Accounts/users.db`, **not** in the previous OS-specific XSoft data directory. Existing accounts in the old directory are **not automatically migrated**. Registration creates a new account in the new drive; do not delete old data before any future migration.

`whoami` displays the logged-in XSoft user; `logout` returns to login; `reboot` restarts MyOs in-process and requires login again; `quit` exits.

## Source structure

- `src/main.cpp`: startup, drive initialization, in-process reboot
- `src/com.cpp`: terminal command routing
- `src/users.cpp`: XSoft user registration, login, persistent password hashes
- `src/filesystem.cpp`: virtual C: path resolution, filesystem commands
- Matching `.hpp` files define interfaces.

External application commands (`store`, `calc`, `games`, `social`, etc.) remain placeholders. GUI, BIOS-style startup, and `.xse` support have not yet been added.

## Verification status

Changes are committed to the development branch and draft pull request. A CTest filesystem regression test and a Linux/Windows GitHub Actions build workflow have been added. Their results are not yet confirmed; do not treat this revision as verified until the workflow completes successfully. The previous standalone account implementation was tested before the virtual-drive revision.

## Tests

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests cover drive initialization, C: path display, file creation/removal, directory navigation, reboot directory reset, traversal rejection, other-drive rejection, and symlink navigation rejection where supported.
