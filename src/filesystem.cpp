#include "filesystem.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace myos {
namespace fs=std::filesystem;
namespace {
std::string ask(const char* message) {
 std::cout<<message<<std::flush;
 std::string input; std::getline(std::cin,input); return input;
}
}
bool handle_filesystem_command(const std::string& command) {
 if(command!="pwd" && command!="ls" && command!="dir" && command!="cd" &&
 command!="mkdir" && command!="mkfile" && command!="touch" && command!="rmdir" &&
 command!="rmfile" && command!="cat") return false;
 try {
 if(command=="pwd") std::cout<<fs::current_path().string()<<'\n';
 else if(command=="ls" || command=="dir") {
  for(const auto& entry:fs::directory_iterator(fs::current_path()))
   std::cout<<(entry.is_directory()?"[DIR] ":"      ")<<entry.path().filename().string()<<'\n';
 } else if(command=="cd") fs::current_path(ask("Directory: "));
 else if(command=="mkdir") fs::create_directory(ask("Directory name: "));
 else if(command=="mkfile" || command=="touch") {
  std::ofstream file(ask("File name: "),std::ios::app);
  if(!file) std::cout<<"Could not create file.\n";
 } else if(command=="rmdir") {
  const fs::path path(ask("Directory to remove: "));
  if(!fs::is_directory(path) || !fs::is_empty(path)) std::cout<<"Directory missing or not empty.\n";
  else fs::remove(path);
 } else if(command=="rmfile") {
  const fs::path path(ask("File to remove: "));
  if(!fs::is_regular_file(path)) std::cout<<"Not a regular file.\n";
  else fs::remove(path);
 } else if(command=="cat") {
  std::ifstream file(ask("File to read: "));
  if(!file) std::cout<<"Could not open file.\n";
  else std::cout<<file.rdbuf();
 }
 } catch(const fs::filesystem_error& e) {
  std::cout<<"Filesystem error: "<<e.code().message()<<'\n';
 }
 return true;
}
}