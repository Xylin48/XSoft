#include "com.hpp"
#include "filesystem.hpp"
#include "users.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
int main() {
 try {
  // A portable installation: the xsoft/C drive lives in the launch directory.
  const auto root=std::filesystem::current_path()/"xsoft"/"C";
  myos::Drive drive(root);
  std::filesystem::create_directories(drive.root()/"MyOs"/"System"/"Accounts");
  std::filesystem::create_directories(drive.root()/"Users");
  std::filesystem::create_directories(drive.root()/"Program Files");
  std::filesystem::create_directories(drive.root()/"Temp");
  myos::Users users(drive.root()/"MyOs"/"System"/"Accounts");
  for(;;) {
   drive.reset();
   std::cout<<"XSoft - booting MyOs\n";
   const auto result=myos::run_shell(users,drive);
   if(result==myos::ShellResult::Shutdown) return 0;
   std::cout<<"MyOs: rebooting...\n";
  }
 } catch(const std::exception& e) {
  std::cerr<<"XSoft startup failed: "<<e.what()<<'\n'; return 1;
 }
}