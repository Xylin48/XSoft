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

XSoft creates a **real folder-backed drive** relative to the directory from which you launch the application:

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

**Portability note:** The storage location is based on the process's *launch working directory*, not the executable's location. Launch XSoft from the same directory each time to access the same C: drive. Future work can bind storage to the installation directory instead.

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

Changes are committed to the development branch and draft pull request. Compilation and end-to-end runtime tests of this latest virtual-drive revision have **not yet been verified**. The previous standalone account implementation was tested before this revision.
