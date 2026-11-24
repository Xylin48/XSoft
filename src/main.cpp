// XSoft entry point. Reboot restarts MyOs inside this process, not the host OS.
#include "com.hpp"
#include <filesystem>
#include <iostream>

int main() {
    const auto startup_directory = std::filesystem::current_path();
    for (;;) {
        // A fresh shell session is a fresh MyOs boot: transient state is reset.
        std::filesystem::current_path(startup_directory);
        std::cout << "XSoft - booting MyOs\n";
        const auto result = myos::run_shell();
        if (result == myos::ShellResult::Shutdown) return 0;
        std::cout << "MyOs: rebooting...\n";
    }
}
