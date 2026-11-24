// Translation of PyOs/Home/com.py: command dispatch and application hub.
// No host reboot, arbitrary shell execution, or privileged package installation.
#include "com.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace myos {
namespace fs = std::filesystem;
namespace {
std::string trim(std::string s) {
    const auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return {};
    const auto b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
std::string ask(const char* prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return trim(value);
}
void banner() {
    std::cout << "\nXSoft - MyOs\n";
    std::cout << "Programs and games hub\n";
}
void not_installed(const std::string& name) {
    std::cout << name << " is not integrated yet. The PyOs source is preserved.\n";
}
void list_dir() {
    for (const auto& e : fs::directory_iterator(fs::current_path()))
        std::cout << (e.is_directory() ? "[DIR] " : "      ")
                  << e.path().filename().string() << '\n';
}
void help() {
    std::cout << "help, quit, reboot, clear, whoami, echo, pwd, cd, ls, dir, "
                 "mkdir, mkfile, touch, rmdir, rmfile, cat, edit, run, bash, "
                 "firefox, games, calc, store, social, mkusr, ip, internet speed\n";
}
}
ShellResult run_shell() {
    banner();
    std::string user = "MyOs user";
    for (;;) {
        std::cout << "\n>>> ";
        std::string command;
        if (!std::getline(std::cin, command)) return ShellResult::Shutdown;
        command = trim(command);
        try {
            if (command.empty()) continue;
            if (command == "quit") return ShellResult::Shutdown;
            if (command == "reboot") return ShellResult::Reboot;
            if (command == "help") { help(); continue; }
            if (command == "clear") { std::cout << "\033[2J\033[H"; continue; }
            if (command == "whoami") { std::cout << user << '\n'; continue; }
            if (command == "mkusr") {
                const auto name = ask("enter username... ");
                if (!name.empty()) user = name;
                continue;
            }
            if (command == "echo") { std::cout << ask(":") << '\n'; continue; }
            if (command == "pwd") { std::cout << fs::current_path().string() << '\n'; continue; }
            if (command == "dir" || command == "ls") { list_dir(); continue; }
            if (command == "cd") {
                fs::current_path(ask("enter the dir you'd like to change to... "));
                continue;
            }
            if (command == "mkdir") {
                fs::create_directory(ask("enter name of dir... "));
                continue;
            }
            if (command == "mkfile" || command == "touch") {
                std::ofstream(ask("enter name of file... "), std::ios::app);
                continue;
            }
            if (command == "rmdir") {
                const auto p = fs::path(ask("enter name of dir you would like to rm... "));
                if (!fs::is_directory(p) || !fs::is_empty(p)) {
                    std::cout << "Directory does not exist or is not empty.\n";
                } else fs::remove(p);
                continue;
            }
            if (command == "rmfile") {
                const auto p = fs::path(ask("enter name of file... "));
                if (!fs::is_regular_file(p)) std::cout << "Not a regular file.\n";
                else fs::remove(p);
                continue;
            }
            if (command == "cat") {
                std::ifstream in(ask("which file would you like to read?... "));
                if (!in) std::cout << "Could not open file.\n";
                else std::cout << in.rdbuf();
                continue;
            }
            if (command == "calc" || command == "games" || command == "store" ||
                command == "social" || command == "firefox" || command == "run" ||
                command == "bash" || command == "edit" || command == "ip" ||
                command == "internet speed") {
                not_installed(command);
                continue;
            }
            std::cout << "Unknown command. Type help.\n";
        } catch (const fs::filesystem_error& e) {
            std::cout << "Filesystem error: " << e.code().message() << '\n';
        }
    }
}
} // namespace myos
