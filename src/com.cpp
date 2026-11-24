#include "com.hpp"
#include "calculator.hpp"
#include "filesystem.hpp"
#include "users.hpp"
#include "tictactoe.hpp"
#include "packages.hpp"
#include "native_xse.hpp"
#include "store.hpp"
#include <iostream>
#include <string>
namespace myos {
namespace {
void banner() { std::cout<<"\nXSoft - MyOs\nPrograms and games hub\n"; }
void help() {
 std::cout<<"help, quit, reboot, logout, whoami, clear, echo, pwd, cd, ls, dir, "
 "mkdir, mkfile, touch, rmdir, rmfile, cat, run, bash, edit, firefox, "
 "games, calc, apps, uninstall, store, social, ip, internet speed\n";
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
  const bool run_command=command.rfind("run ",0)==0;
  const bool direct_xse=command.size()>4 && command.substr(command.size()-4)==".xse";
  if(run_command || direct_xse) {
   try {
    const auto target=drive.executable(run_command?command.substr(4):command,run_command);
    const int status=native_xse::launch(target);
    if(status!=0) std::cout<<"Application exit code: "<<status<<'\n';
   } catch(const std::exception& e) { std::cout<<"Launch failed: "<<e.what()<<'\n'; }
   continue;
  }
  if(command=="store" || command.rfind("store ",0)==0) {
   try {
    std::string id;
    if(command=="store") { std::cout<<"Application name (blank to cancel): "<<std::flush; if(!std::getline(std::cin,id)||id.empty()) continue; }
    else id=command.substr(6);
    auto result=store::download(drive.root(),id);
    std::cout<<"Installed "<<id<<".xse\n";
    if(result.cleanup_error) std::cout<<"Temporary cleanup failed: "<<result.cleanup_error.message()<<'\n';
   } catch(const std::exception& e) { std::cout<<"Store error: "<<e.what()<<'\n'; }
   continue;
  }
  if(command.rfind("uninstall ",0)==0) {
   try {
    const auto id=command.substr(10);
    if(!xse::valid_id(id)) throw std::runtime_error("Invalid application identifier");
    const auto target=drive.executable("C:/Apps/"+id+".xse",false);
    if(!std::filesystem::remove(target)) throw std::runtime_error("Removal failed");
    std::cout<<"Removed "<<id<<".xse\n";
   } catch(const std::exception& e) { std::cout<<"Removal failed: "<<e.what()<<'\n'; }
   continue;
  }
  if(command=="apps") {
   try {
    const auto apps=xse::installed_packages(drive.root()/"Apps");
    for(const auto& app:apps) std::cout<<app.id<<'\n';
    if(apps.empty()) std::cout<<"No installed XSE applications.\n";
   } catch(const std::exception& e) { std::cout<<"Application discovery failed: "<<e.what()<<'\n'; }
   continue;
  }
  if(drive.command(command)) continue;
  if(command=="calc") { run_calculator(std::cin,std::cout); continue; }
  if(command=="games") { run_tictactoe(std::cin,std::cout); continue; }
  if(command=="social" || command=="firefox" || command=="run" ||
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
