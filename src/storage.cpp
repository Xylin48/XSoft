#include "storage.hpp"
#include <filesystem>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <array>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <vector>
#endif
namespace myos {
namespace fs=std::filesystem;
fs::path installation_directory(const char* argv0) {
#ifdef _WIN32
 std::wstring buffer(32768,L'\0');
 const DWORD length=GetModuleFileNameW(nullptr,buffer.data(),static_cast<DWORD>(buffer.size()));
 if(length==0||length>=buffer.size()) throw std::runtime_error("Cannot locate XSoft executable");
 buffer.resize(length);
 return fs::path(buffer).parent_path();
#elif defined(__linux__)
 std::array<char,4096> buffer{};
 const auto length=readlink("/proc/self/exe",buffer.data(),buffer.size());
 if(length>0&&static_cast<size_t>(length)<buffer.size())
  return fs::path(std::string(buffer.data(),static_cast<size_t>(length))).parent_path();
#elif defined(__APPLE__)
 uint32_t length=0;
 _NSGetExecutablePath(nullptr,&length);
 std::vector<char> buffer(length+1);
 if(_NSGetExecutablePath(buffer.data(),&length)==0)
  return fs::canonical(fs::path(buffer.data())).parent_path();
#endif
 if(!argv0||!*argv0) throw std::runtime_error("Cannot locate XSoft executable");
 const fs::path candidate(argv0);
 if(candidate.has_parent_path()) return fs::absolute(candidate).lexically_normal().parent_path();
 throw std::runtime_error("Cannot locate XSoft executable; use an explicit executable path");
}
fs::path drive_directory(const char* argv0) {
 return installation_directory(argv0)/"xsoft"/"C";
}
void initialize_drive(const fs::path& root) {
 // Do not follow pre-existing symlinks for any system directory.
 for(const auto& relative:{fs::path("MyOs"),fs::path("MyOs/System"),
     fs::path("MyOs/System/Accounts"),fs::path("Users"),
     fs::path("Program Files"),fs::path("Temp"),fs::path("apps")}) {
  const auto path=root/relative;
  if(fs::is_symlink(fs::symlink_status(path)))
   throw std::runtime_error("XSoft system directory is a symlink: "+path.string());
  fs::create_directory(path);
  if(!fs::is_directory(path)) throw std::runtime_error("Invalid XSoft directory: "+path.string());
 }
}
}