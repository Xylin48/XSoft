# XSoft / MyOs

XSoft hosts **MyOs**, the C++ successor to PyOs: a lightweight terminal-based application and game hub. This is a work-in-progress port, not a PC emulator or graphical desktop.

## Build

Requires a C++17 compiler, CMake 3.12+, and **OpenSSL Crypto** development libraries.

```sh
cmake -S . -B build
cmake --build build
./build/xsoft
```

On Windows, run the executable produced by the selected CMake generator (for example `build/Debug/xsoft.exe`).

## Standalone XSoft accounts

At startup choose `register`, `login`, or `quit`. Registration asks for a username and password. Passwords are salted using cryptographic randomness and hashed with PBKDF2-HMAC-SHA256 (310,000 iterations); plaintext passwords are not saved.

The local account database is stored at `$XDG_DATA_HOME/XSoft/users.db` if XDG_DATA_HOME is set, otherwise `$HOME/XSoft/users.db` on Unix. On Windows it uses `%LOCALAPPDATA%\\XSoft\\users.db` (falling back to USERPROFILE). This is **local, offline authentication**, not a network identity service, and it does not use Replit.

Commands: `whoami` shows the logged-in XSoft user; `logout` returns to the login/register prompt; `reboot` restarts MyOs within XSoft, resets the session, and requires login again; `quit` exits XSoft.

## Current translated subsystems

- `src/main.cpp` — host application entry point and in-process MyOs reboot loop
- `src/com.cpp` — interactive terminal and command router
- `src/users.cpp` — account registration, login, logout and persistent user database (from PyOs `Home/usr.py`)
- `src/filesystem.cpp` — `pwd`, `cd`, `ls`, `dir`, `mkdir`, `mkfile`, `touch`, `rmdir`, `rmfile`, `cat`
- Matching `.hpp` headers keep interfaces separate.

**Important:** Filesystem commands currently operate on the host process's working directory and are **not sandboxed**; avoid running XSoft in a sensitive directory. External app commands (`store`, `calc`, `games`, `social`, etc.) are placeholders pending later integration. GUI, BIOS-style startup, and `.xse` support have not been added.

## Scope

Only PyOs `Home/main.py`, `Home/com.py`, and `Home/usr.py` functionality has been ported so far, with filesystem logic extracted as its own module. The original PyOs source remains untouched.
