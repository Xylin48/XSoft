#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <stdexcept>
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
}
