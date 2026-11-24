#include "../src/installer.hpp"
#include <iostream>
#include <random>
using namespace myos::xse;
int main() {
 fs::path root;
 try {
  std::random_device random;
  for(int i=0;i<32;++i) {
   auto p=fs::temp_directory_path()/("xsoft-test-"+std::to_string(random())+"-"+std::to_string(random()));
   if(fs::create_directory(p)){root=p;break;}
  }
  if(root.empty()) throw std::runtime_error("Cannot create test directory");
  auto require=[](bool ok){if(!ok) throw std::runtime_error("Assertion failed");};
  auto rejects=[&](auto fn){bool rejected=false;try{fn();}catch(const std::exception&){rejected=true;}require(rejected);};
  Package p{"demo",{1,2,3}};
  auto bytes=encode(p);
  require(decode(bytes).id==p.id && decode(bytes).payload==p.payload);
  auto corrupt=bytes;corrupt[12]^=1;
  rejects([&]{decode(corrupt);});
  rejects([&]{encode({"../escape",{1}});});
  auto result=install_new(root,p);
  require(!result.cleanup_error && read_package(result.installed)==bytes);
  rejects([&]{install_new(root,{"demo",{9}});});
  require(read_package(result.installed)==bytes);
  std::size_t count=0;
  for(const auto& entry:fs::directory_iterator(root)){require(entry.path()==result.installed);++count;}
  require(count==1);
  fs::remove_all(root);return 0;
 } catch(const std::exception& e) {
  std::cerr<<e.what()<<'\n';
  std::error_code ec;if(!root.empty())fs::remove_all(root,ec);return 1;
 }
}
