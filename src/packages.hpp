#pragma once
#include "installer.hpp"
#include <vector>
namespace myos::xse {
struct InstalledPackage { std::string id; fs::path path; };
inline std::vector<InstalledPackage> installed_packages(const fs::path& apps) {
 std::vector<InstalledPackage> result;
 if(!fs::exists(apps)) return result;
 if(fs::is_symlink(fs::symlink_status(apps))) throw std::runtime_error("Invalid application directory");
 for(const auto& entry:fs::directory_iterator(apps)) {
  if(entry.path().extension()!=".xse" || !fs::is_regular_file(entry.symlink_status())) continue;
  auto id=entry.path().stem().string();
  if(!valid_id(id)) continue;
  result.push_back({id,entry.path()});
 }
 std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.id<b.id;});
 return result;
}
}
