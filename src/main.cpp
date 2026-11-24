#include "com.hpp"
#include "filesystem.hpp"
#include "storage.hpp"
#include "users.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv) {
 try {
  (void)argc;
  const auto root=myos::drive_directory(argv[0]);
  myos::Drive drive(root);
  myos::initialize_drive(drive.root());
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