#include "filesystem.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace myos {
namespace fs=std::filesystem;
namespace {
std::string ask(const char* message) {
 std::cout<<message<<std::flush;
 std::string input; std::getline(std::cin,input); return input;
}
bool is_drive_prefix(const std::string& s) {
 return s.size()>=2 && std::isalpha(static_cast<unsigned char>(s[0])) && s[1]==':';
}
}
Drive::Drive(fs::path root):root_(fs::absolute(std::move(root)).lexically_normal()) {
 // Refuse existing symbolic links in the storage root and all its ancestors.
 for(fs::path p=root_; !p.empty(); p=p.parent_path()) {
  if(fs::is_symlink(fs::symlink_status(p))) throw std::runtime_error("XSoft storage path contains a symlink");
  if(p==p.root_path()) break;
 }
 fs::create_directories(root_);
 if(!fs::is_directory(root_)) throw std::runtime_error("XSoft C: is not a directory");
 reset();
}
void Drive::reset() { cwd_.clear(); }
std::string Drive::cwd_display() const {
 std::string result="C:\\";
 for(const auto& component:cwd_) {
  if(component=="." || component.empty()) continue;
  if(result.back()!='\\') result+='\\';
  result+=component.string();
 }
 return result;
}
fs::path Drive::resolve(const std::string& input) const {
 if(input.empty()) throw std::runtime_error("A path is required");
 std::string value=input;
 std::replace(value.begin(),value.end(),'\\','/');
 bool absolute=false;
 if(is_drive_prefix(value)) {
  if(std::toupper(static_cast<unsigned char>(value[0]))!='C')
   throw std::runtime_error("Only C: is available");
  value.erase(0,2);
  if(value.empty()) throw std::runtime_error("A path is required");
  absolute=true;
 }
 if(!value.empty()&&value[0]=='/') absolute=true;
 fs::path relative=absolute?fs::path{}:cwd_;
 // Explicit component processing prevents lexical traversal outside C:.
 size_t pos=0;
 while(pos<=value.size()) {
  const auto end=value.find('/',pos);
  const auto part=value.substr(pos,end==std::string::npos?std::string::npos:end-pos);
  if(part=="..") {
   if(relative.empty()) throw std::runtime_error("Cannot leave C: drive");
   relative=relative.parent_path();
  } else if(!part.empty()&&part!=".") {
   if(part.find(':')!=std::string::npos) throw std::runtime_error("Invalid drive path");
   relative/=part;
  }
  if(end==std::string::npos) break;
  pos=end+1;
 }
 const fs::path target=(root_/relative).lexically_normal();
 // Reject symlinks in every existing component, including the final file.
 fs::path walk=root_;
 for(const auto& part:relative) {
  walk/=part;
  const auto status=fs::symlink_status(walk);
  if(fs::is_symlink(status)) throw std::runtime_error("Symlinks are not supported in C:");
 }
 return target;
}
fs::path Drive::executable(const std::string& name,bool search) const {
 if(name.empty()) throw std::runtime_error("Executable name required");
 auto file=name;
 if(fs::path(file).extension()!=".xse") file+=".xse";
 auto target=resolve(file);
 if(fs::is_regular_file(target)) return target;
 if(search && file.find_first_of("/\\:")==std::string::npos) {
  target=resolve("C:/Apps/"+file);
  if(fs::is_regular_file(target)) return target;
 }
 throw std::runtime_error("XSE executable not found");
}
bool Drive::command(const std::string& command) {
 if(command!="pwd" && command!="ls" && command!="dir" && command!="cd" &&
 command!="mkdir" && command!="mkfile" && command!="touch" && command!="rmdir" &&
 command!="rmfile" && command!="cat") return false;
 try {
  if(command=="pwd") std::cout<<cwd_display()<<'\n';
  else if(command=="ls"||command=="dir") {
   for(const auto& entry:fs::directory_iterator(resolve("."))) {
    if(entry.is_symlink()) {
     std::cout<<"[LINK] "<<entry.path().filename().string()<<" (inaccessible)\n";
    } else {
     std::cout<<(entry.is_directory()?"[DIR] ":"      ")
              <<entry.path().filename().string()<<'\n';
    }
   }
  } else if(command=="cd") {
   const auto path=resolve(ask("Directory: "));
   if(!fs::is_directory(path)) std::cout<<"Directory not found.\n";
   else {
    cwd_=path.lexically_relative(root_);
    if(cwd_==".") cwd_.clear();
   }
  } else if(command=="mkdir") {
   if(!fs::create_directory(resolve(ask("Directory name: "))))
    std::cout<<"Directory already exists.\n";
  } else if(command=="mkfile"||command=="touch") {
   std::ofstream file(resolve(ask("File name: ")),std::ios::app);
   if(!file) std::cout<<"Could not create file.\n";
  } else if(command=="rmdir") {
   const auto path=resolve(ask("Directory to remove: "));
   if(path==root_) std::cout<<"Cannot remove C: root.\n";
   else if(!fs::is_directory(path)||!fs::is_empty(path))
    std::cout<<"Directory missing or not empty.\n";
   else fs::remove(path);
  } else if(command=="rmfile") {
   const auto path=resolve(ask("File to remove: "));
   if(!fs::is_regular_file(path)) std::cout<<"Not a regular file.\n";
   else fs::remove(path);
  } else if(command=="cat") {
   std::ifstream file(resolve(ask("File to read: ")));
   if(!file) std::cout<<"Could not open file.\n";
   else std::cout<<file.rdbuf();
  }
 } catch(const fs::filesystem_error& e) {
  std::cout<<"Filesystem error: "<<e.code().message()<<'\n';
 } catch(const std::runtime_error& e) {
  std::cout<<"Path error: "<<e.what()<<'\n';
 }
 return true;
}
}