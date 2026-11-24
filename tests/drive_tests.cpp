#include "../src/filesystem.hpp"
#include "../src/storage.hpp"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
namespace fs=std::filesystem;
namespace {
struct Input {
 std::streambuf* old;
 std::istringstream stream;
 explicit Input(const std::string& text):old(std::cin.rdbuf()),stream(text) {
  std::cin.rdbuf(stream.rdbuf());
 }
 ~Input() { std::cin.rdbuf(old); std::cin.clear(); }
};
struct Output {
 std::streambuf* old;
 std::ostringstream stream;
 Output():old(std::cout.rdbuf(stream.rdbuf())) {}
 ~Output() { std::cout.rdbuf(old); }
};
void run(myos::Drive& drive,const std::string& cmd,const std::string& input) {
 Input in(input+"\n");
 Output out;
 if(!drive.command(cmd)) throw std::runtime_error("Command not handled: "+cmd);
}
void check(bool condition,const char* message) {
 if(!condition) throw std::runtime_error(message);
}
}
int main() {
 const auto unique=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
 const fs::path base=fs::temp_directory_path()/("xsoft-drive-test-"+unique);
 try {
  myos::Drive drive(base/"xsoft"/"C");
  myos::initialize_drive(drive.root());
  check(fs::is_directory(drive.root()/"MyOs/System/Accounts"),"Missing account directory");
  check(drive.cwd_display()=="C:\\","Drive must start at C root");
  run(drive,"mkdir","Documents");
  check(fs::is_directory(drive.root()/"Documents"),"mkdir failed");
  run(drive,"cd","Documents");
  check(drive.cwd_display()=="C:\\Documents","cd display incorrect");
  run(drive,"touch","notes.txt");
  check(fs::is_regular_file(drive.root()/"Documents"/"notes.txt"),"touch failed");
  run(drive,"cd","..");
  check(drive.cwd_display()=="C:\\","cd .. failed");
  run(drive,"cd","C:\\Documents");
  check(drive.cwd_display()=="C:\\Documents","Absolute C: path failed");
  run(drive,"cd","../../..");
  check(drive.cwd_display()=="C:\\Documents","Traversal changed directory");
  run(drive,"cd","D:\\");
  check(drive.cwd_display()=="C:\\Documents","Foreign drive accepted");
  drive.reset();
  check(drive.cwd_display()=="C:\\","Reboot reset failed");
  run(drive,"rmfile","Documents/notes.txt");
  check(!fs::exists(drive.root()/"Documents"/"notes.txt"),"rmfile failed");
  run(drive,"rmdir","Documents");
  check(!fs::exists(drive.root()/"Documents"),"rmdir failed");
  // Symlinks should not be followed into the host filesystem.
  std::error_code ec;
  fs::create_directory_symlink(fs::temp_directory_path(),drive.root()/"escape",ec);
  if(!ec) {
   run(drive,"cd","escape");
   check(drive.cwd_display()=="C:\\","Symlink escape accepted");
  }
  fs::remove_all(base);
  std::cout<<"XSoft drive tests passed\n";
  return 0;
 } catch(const std::exception& e) {
  std::cerr<<"XSoft drive test failed: "<<e.what()<<'\n';
  std::error_code ec; fs::remove_all(base,ec); return 1;
 }
}