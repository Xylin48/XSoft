#pragma once
#include "native_xse.hpp"
namespace myos::store {
namespace fs=std::filesystem;
inline std::vector<std::string> catalog(const fs::path& root) {
 std::random_device random;
 auto stage=root/(".catalog-"+std::to_string(random())+"-"+std::to_string(random()));
 if(!fs::create_directory(stage)) throw std::runtime_error("Cannot create catalog staging");
 try {
  auto repo=fs::absolute(stage/"repo");
  if(native_xse::process("git",{"clone","--bare","--filter=blob:none","--no-tags","https://github.com/Xylin48/Store.git",repo.string()})!=0)
   throw std::runtime_error("Cannot retrieve catalog");
  std::vector<std::string> names;
  auto refs=repo/"refs"/"heads";
  if(fs::exists(refs)) for(const auto& e:fs::directory_iterator(refs)) {
   auto id=e.path().filename().string();
   if(xse::valid_id(id)&&id!="main"&&id!="master") names.push_back(id);
  }
  std::ifstream packed(repo/"packed-refs"); std::string line;
  while(std::getline(packed,line)){
   auto pos=line.find(" refs/heads/");
   if(pos==std::string::npos) continue;
   auto id=line.substr(pos+12);
   if(xse::valid_id(id)&&id!="main"&&id!="master") names.push_back(id);
  }
  std::sort(names.begin(),names.end()); names.erase(std::unique(names.begin(),names.end()),names.end());
  fs::remove_all(stage); return names;
 } catch(...) {std::error_code ec;fs::remove_all(stage,ec);throw;}
}
inline xse::InstallResult download(const fs::path& root,const std::string& id) {
 if(!xse::valid_id(id)) throw std::runtime_error("Invalid Store application name");
 std::random_device random; fs::path stage;
 for(int i=0;i<32;++i) {
  auto candidate=root/(".store-"+std::to_string(random())+"-"+std::to_string(random()));
  if(fs::create_directory(candidate)){stage=candidate;break;}
 }
 if(stage.empty()) throw std::runtime_error("Cannot create Store staging directory");
 try {
  auto source=fs::absolute(stage/"source");
  if(native_xse::process("git",{"clone","--depth","1","--single-branch","--branch",id,"--","https://github.com/Xylin48/Store.git",source.string()})!=0)
   throw std::runtime_error("Store download failed");
  fs::remove_all(source/".git");
  auto result=native_xse::install_source(root/"apps",id,source);
  std::error_code cleanup; fs::remove_all(stage,cleanup);
  if(cleanup && !result.cleanup_error) result.cleanup_error=cleanup;
  return result;
 } catch(...) {
  std::error_code cleanup; fs::remove_all(stage,cleanup);
  if(cleanup) throw std::runtime_error("Store installation failed; download cleanup incomplete");
  throw;
 }
}
}
