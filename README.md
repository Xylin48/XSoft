# XSoft

XSoft hosts **MyOs**, the successor to PyOs: a lightweight application and game hub.

Initial port scope: only `Home/main.py` and `Home/com.py` from `Xylin48/PyOs`, translated into C++ (`src/main.cpp` and `src/com.cpp`). No BIOS, GUI, Store implementation, or other PyOs files are ported in this change.

Build (C++17):

```sh
cmake -S . -B build
cmake --build build
./build/xsoft
```

The current prototype runs in a host terminal. The `reboot` command restarts the MyOs command session, never the host computer. Commands launching unported Python apps report that those applications are not yet integrated.
