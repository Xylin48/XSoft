#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
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
inline int process(const std::string& executable,const std::vector<std::string>& args,const fs::path& cwd={}) {
 std::vector<const char*> argv{executable.c_str()};
 for(const auto& arg:args) argv.push_back(arg.c_str());
 argv.push_back(nullptr);
#ifdef _WIN32
 auto previous=fs::current_path();
 if(!cwd.empty()) fs::current_path(cwd);
 auto status=_spawnvp(_P_WAIT,executable.c_str(),argv.data());
 if(!cwd.empty()) fs::current_path(previous);
 if(status==-1) throw std::runtime_error("Cannot start process");
 return static_cast<int>(status);
#else
 std::vector<char*> av; for(auto p:argv) av.push_back(const_cast<char*>(p));
 auto pid=fork(); if(pid<0) throw std::runtime_error("Cannot fork process");
 if(pid==0){if(!cwd.empty() && chdir(cwd.c_str())!=0)_exit(126);execvp(executable.c_str(),av.data());_exit(127);}
 int status=0; while(waitpid(pid,&status,0)<0){if(errno!=EINTR)throw std::runtime_error("Cannot wait for process");}
 return WIFEXITED(status)?WEXITSTATUS(status):128;
#endif
}
inline int launch(const fs::path& executable) {
 if(executable.extension()!=".xse" || !fs::is_regular_file(fs::symlink_status(executable)))
  throw std::runtime_error("XSE executable not found");
 auto path=fs::absolute(executable);
 return process(path.string(),{},path.parent_path());
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
  auto output=fs::absolute(stage/"app.xse");
  std::vector<std::string> args{"-std=c++17"};
  if(fs::is_directory(fs::symlink_status(source))) {
   auto tree=stage/"source"; fs::create_directory(tree);
   for(const auto& entry:fs::recursive_directory_iterator(source)) {
    auto status=entry.symlink_status();
    auto target=tree/entry.path().lexically_relative(source);
    if(fs::is_directory(status)) fs::create_directory(target);
    else if(fs::is_regular_file(status)) {
     fs::copy_file(entry.path(),target);
     if(target.extension()==".cpp") args.push_back(fs::absolute(target).string());
    } else throw std::runtime_error("Unsupported source tree entry");
   }
  } else {
   if(!fs::is_regular_file(fs::symlink_status(source))) throw std::runtime_error("Invalid source file");
   auto input=fs::absolute(stage/"main.cpp"); fs::copy_file(source,input);
   args.push_back(input.string());
  }
  if(args.size()==1) throw std::runtime_error("No C++ source files");
  args.push_back("-o"); args.push_back(output.string());
#ifdef _WIN32
  if(process("g++",args)!=0)
#else
  if(process("c++",args)!=0)
#endif
   throw std::runtime_error("XSE compilation failed");
  if(!fs::is_regular_file(fs::symlink_status(output)) || fs::file_size(output)==0) throw std::runtime_error("Compiler produced no executable");
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
