#include "store.hpp"
#include "packages.hpp"
#include "storage.hpp"
#include <iostream>
int main(int argc,char** argv) {
 try {
  (void)argc;
  const auto root=myos::installation_directory(argv[0]).parent_path();
  for(;;) {
   std::cout<<"\nXSoft Store\n1. Browse applications\n2. Download application\n3. Installed applications\n4. Remove application\n5. Exit\nSelect: "<<std::flush;
   std::string choice,id;
   if(!std::getline(std::cin,choice)||choice=="5") return 0;
   try {
    if(choice=="1") {
     if(myos::native_xse::process("git",{"ls-remote","--heads","https://github.com/Xylin48/Store.git"})!=0)
      throw std::runtime_error("Cannot retrieve Store catalog");
    } else if(choice=="2") {
     std::cout<<"Application name: "<<std::flush;
     if(!std::getline(std::cin,id)) return 0;
     auto result=myos::store::download(root,id);
     std::cout<<"Installed "<<id<<".xse\n";
     if(result.cleanup_error) std::cout<<"Cleanup failed: "<<result.cleanup_error.message()<<'\n';
    } else if(choice=="3") {
     for(const auto& app:myos::xse::installed_packages(root/"apps")) std::cout<<app.id<<'\n';
    } else if(choice=="4") {
     std::cout<<"Application name: "<<std::flush;
     if(!std::getline(std::cin,id)) return 0;
     if(!myos::xse::valid_id(id)||id=="store") throw std::runtime_error("Invalid removal target");
     for(const auto& app:myos::xse::installed_packages(root/"apps"))
      if(app.id==id) { std::filesystem::remove(app.path); std::cout<<"Removed "<<id<<'\n'; }
    }
   } catch(const std::exception& e){std::cout<<"Store error: "<<e.what()<<'\n';}
  }
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
