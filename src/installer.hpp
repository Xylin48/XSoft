#pragma once
#include "xse.hpp"
#include <chrono>
#include <random>
namespace myos::xse {
namespace fs = std::filesystem;
inline Bytes read_package(const fs::path& path) {
 std::ifstream in(path,std::ios::binary|std::ios::ate);
 if(!in) throw std::runtime_error("Cannot open package");
 auto n=in.tellg();
 if(n<0 || n>static_cast<std::streamoff>(limit+108)) throw std::runtime_error("Package too large");
 Bytes b(static_cast<std::size_t>(n)); in.seekg(0);
 if(!in.read(reinterpret_cast<char*>(b.data()),static_cast<std::streamsize>(b.size())))
  throw std::runtime_error("Cannot read package");
 return b;
}
struct InstallResult { fs::path installed; std::error_code cleanup_error; };
// Caller supplies a trusted, private application directory under the virtual drive.
// Input payload is already prepared; this function owns only its newly created staging directory.
inline InstallResult install_new(const fs::path& apps,const Package& package) {
 auto bytes=encode(package);
 fs::create_directories(apps);
 if(fs::is_symlink(fs::symlink_status(apps))) throw std::runtime_error("Invalid application directory");
 auto dest=apps/(package.id+".xse");
 std::random_device random;
 fs::path stage;
 for(unsigned attempt=0;attempt<32;++attempt) {
  auto candidate=apps/(".install-"+std::to_string(random())+"-"+std::to_string(random()));
  if(fs::create_directory(candidate)){stage=candidate;break;}
 }
 if(stage.empty()) throw std::runtime_error("Cannot allocate staging directory");
 auto staged=stage/"package.xse";
 bool placed=false;
 try {
  std::ofstream out(staged,std::ios::binary|std::ios::trunc);
  out.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
  out.close();
  if(!out) throw std::runtime_error("Cannot write staged package");
  if(read_package(staged)!=bytes) throw std::runtime_error("Staged package verification failed");
  // Hard link publishes a complete file without replacing an existing destination.
  // Both paths reside on the same filesystem. Unsupported filesystems fail cleanly.
  fs::create_hard_link(staged,dest); placed=true;
  if(read_package(dest)!=bytes) throw std::runtime_error("Installed package verification failed");
 } catch(...) {
  std::error_code rollback,cleanup;
  if(placed) fs::remove(dest,rollback);
  fs::remove_all(stage,cleanup);
  if(rollback||cleanup) throw std::runtime_error("Installation failed; rollback or staging cleanup incomplete");
  throw;
 }
 std::error_code cleanup;
 fs::remove_all(stage,cleanup);
 return {dest,cleanup};
}
}
