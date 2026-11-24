#include "../src/native_xse.hpp"
#include "../src/packages.hpp"
#include "../src/filesystem.hpp"
#include <iostream>
#include <fstream>
using namespace myos;
namespace fs=std::filesystem;
int main(){
 fs::path root;
 try {
  std::random_device random;
  for(int i=0;i<32;++i){auto p=fs::temp_directory_path()/("xsoft-test-"+std::to_string(random())+"-"+std::to_string(random()));if(fs::create_directory(p)){root=p;break;}}
  if(root.empty()) throw std::runtime_error("Cannot create test directory");
  auto require=[](bool ok){if(!ok)throw std::runtime_error("Assertion failed");};
  auto rejects=[&](auto fn){bool rejected=false;try{fn();}catch(const std::exception&){rejected=true;}require(rejected);};
  auto source=root/"demo.cpp"; std::ofstream(source)<<"int main(){return 23;}";
  auto apps=root/"Apps";
  auto result=native_xse::install_source(apps,"demo",source);
  require(!result.cleanup_error && native_xse::launch(result.installed)==23);
  require(xse::installed_packages(apps).size()==1);
  Drive drive(root);
  require(drive.executable("demo",true)==result.installed);
  require(drive.executable("C:/Apps/demo.xse",false)==result.installed);
  rejects([&]{drive.executable("../demo",true);});
  rejects([&]{native_xse::install_source(apps,"demo",source);});
  std::ofstream(source)<<"not valid C++";
  rejects([&]{native_xse::install_source(apps,"broken",source);});
  require(native_xse::launch(result.installed)==23);
  for(const auto& entry:fs::directory_iterator(apps))require(entry.path()==result.installed);
  fs::rename(result.installed,root/"demo.xse");
  require(drive.executable("demo.xse",false)==root/"demo.xse");
  require(native_xse::launch(drive.executable("demo",true))==23);
  fs::remove_all(root);return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';std::error_code ec;if(!root.empty())fs::remove_all(root,ec);return 1;}
}
