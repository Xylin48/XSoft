// Translation of PyOs/Home/main.py.
// The original attempted "import com.py" and read an undefined Replit db.
// XSoft now initializes the command hub explicitly.
#include "com.hpp"
#include <iostream>

int main() {
    std::cout << "XSoft - starting MyOs\n";
    return myos::run_shell();
}
