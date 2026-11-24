#pragma once
#include "xse.hpp"
#include <filesystem>
#include <random>
#include <stdexcept>
namespace myos::xse {
namespace fs=std::filesystem;
struct InstallResult { fs::path installed; std::error_code cleanup_error; };
}
