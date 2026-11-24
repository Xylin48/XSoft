#include "com.hpp"
#include "users.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
namespace {
std::filesystem::path data_directory() {
#ifdef _WIN32
 const char* base=std::getenv("LOCALAPPDATA");
 if(!base || !*base) base=std::getenv("USERPROFILE");
#else
 const char* base=std::getenv("XDG_DATA_HOME");
 if(!base || !*base) base=std::getenv("HOME");
#endif
 if(!base || !*base) throw std::runtime_error("No user data directory available");
 return std::filesystem::path(base)/"XSoft";
}
}
int main() {
 try {
  const auto startup_directory=std::filesystem::current_path();
  myos::Users users(data_directory());
  for(;;) {
   std::filesystem::current_path(startup_directory);
   std::cout<<"XSoft - booting MyOs\n";
   const auto result=myos::run_shell(users);
   if(result==myos::ShellResult::Shutdown) return 0;
   std::cout<<"MyOs: rebooting...\n";
  }
 } catch(const std::exception& e) {
  std::cerr<<"XSoft startup failed: "<<e.what()<<'\n'; return 1;
 }
}