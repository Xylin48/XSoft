#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <stdexcept>
#include "installer.hpp"
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#include <cerrno>
#endif
namespace myos::native_xse {
namespace fs=std::filesystem;
inline int process(const std::string& executable,const std::vector<std::string>& args) {
 std::vector<const char*> argv{executable.c_str()};
 for(const auto& arg:args) argv.push_back(arg.c_str());
 argv.push_back(nullptr);
#ifdef _WIN32
 auto status=_spawnvp(_P_WAIT,executable.c_str(),argv.data());
 if(status==-1) throw std::runtime_error("Cannot start process");
 return static_cast<int>(status);
#else
 std::vector<char*> av; for(auto p:argv) av.push_back(const_cast<char*>(p));
 auto pid=fork(); if(pid<0) throw std::runtime_error("Cannot fork process");
 if(pid==0){execvp(executable.c_str(),av.data());_exit(127);}
 int status=0; while(waitpid(pid,&status,0)<0){if(errno!=EINTR)throw std::runtime_error("Cannot wait for process");}
 return WIFEXITED(status)?WEXITSTATUS(status):128;
#endif
}
inline int launch(const fs::path& executable) {
 if(executable.extension()!=".xse" || !fs::is_regular_file(fs::symlink_status(executable)))
  throw std::runtime_error("XSE executable not found");
 return process(fs::absolute(executable).string(),{});
}
inline xse::InstallResult install_source(const fs::path& apps,const std::string& id,const fs::path& source) {
 if(!xse::valid_id(id)) throw std::runtime_error("Invalid application identifier");
 fs::create_directories(apps);
 if(fs::is_symlink(fs::symlink_status(apps))) throw std::runtime_error("Invalid application directory");
 std::random_device random; fs::path stage;
 for(int i=0;i<32;++i){auto p=apps/(".native-"+std::to_string(random())+"-"+std::to_string(random()));if(fs::create_directory(p)){stage=p;break;}}
 if(stage.empty()) throw std::runtime_error("Cannot create staging directory");
 auto dest=apps/(id+".xse"); bool placed=false;
 try {
  auto input=fs::absolute(stage/"main.cpp"); auto output=fs::absolute(stage/"app.xse");
  fs::copy_file(source,input);
#ifdef _WIN32
  if(process("g++",{"-std=c++17",input.string(),"-o",output.string()})!=0)
#else
  if(process("c++",{"-std=c++17",input.string(),"-o",output.string()})!=0)
#endif
   throw std::runtime_error("XSE compilation failed");
  fs::create_hard_link(output,dest); placed=true;
  if(!fs::is_regular_file(fs::symlink_status(dest))) throw std::runtime_error("Placement failed");
 } catch(...) {
  std::error_code rollback,cleanup;
  if(placed) fs::remove(dest,rollback);
  fs::remove_all(stage,cleanup);
  if(rollback||cleanup) throw std::runtime_error("Rollback or cleanup incomplete");
  throw;
 }
 std::error_code cleanup; fs::remove_all(stage,cleanup); return {dest,cleanup};
}
}
