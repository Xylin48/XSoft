#pragma once
#include <filesystem>
namespace myos {
std::filesystem::path installation_directory(const char* argv0);
std::filesystem::path drive_directory(const char* argv0);
void initialize_drive(const std::filesystem::path& root);
}