#include "com.hpp"
#include "calculator.hpp"
#include "filesystem.hpp"
#include "users.hpp"
#include "tictactoe.hpp"
#include <iostream>
#include <string>
namespace myos {
namespace {
void banner() { std::cout<<"\nXSoft - MyOs\nPrograms and games hub\n"; }
void help() {
 std::cout<<"help, quit, reboot, logout, whoami, clear, echo, pwd, cd, ls, dir, "
 "mkdir, mkfile, touch, rmdir, rmfile, cat, run, bash, edit, firefox, "
 "games, calc, store, social, ip, internet speed\n";
}
}
ShellResult run_shell(Users& users,Drive& drive) {
 banner();
 if(!users.authenticate()) return ShellResult::Shutdown;
 for(;;) {
  std::cout<<"\n"<<drive.cwd_display()<<"> "<<std::flush;
  std::string command;
  if(!std::getline(std::cin,command)) return ShellResult::Shutdown;
  const auto first=command.find_first_not_of(" \t\r\n");
  if(first==std::string::npos) continue;
  const auto last=command.find_last_not_of(" \t\r\n");
  command=command.substr(first,last-first+1);
  if(command=="quit") return ShellResult::Shutdown;
  if(command=="reboot") { users.logout(); return ShellResult::Reboot; }
  if(command=="logout") {
   users.logout(); drive.reset();
   if(!users.authenticate()) return ShellResult::Shutdown;
   continue;
  }
  if(command=="help") { help(); continue; }
  if(command=="whoami") { std::cout<<users.current()<<'\n'; continue; }
  if(command=="clear") { std::cout<<"\033[2J\033[H"; continue; }
  if(command=="echo") {
   std::cout<<":"<<std::flush;
   std::string message; std::getline(std::cin,message);
   std::cout<<message<<'\n'; continue;
  }
  if(drive.command(command)) continue;
  if(command=="calc") { run_calculator(std::cin,std::cout); continue; }
  if(command=="games") { run_tictactoe(std::cin,std::cout); continue; }
  if(command=="store" ||
     command=="social" || command=="firefox" || command=="run" ||
     command=="bash" || command=="edit" || command=="ip" ||
     command=="internet speed") {
   std::cout<<command<<" is not integrated yet.\n"; continue;
  }
  if(command=="mkusr" || command=="register" || command=="login") {
   std::cout<<"Use logout to switch accounts; login and registration are available at startup.\n";
   continue;
  }
  std::cout<<"Unknown command. Type help.\n";
 }
}
}
