#include "users.hpp"
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <conio.h>
#include <io.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#endif
namespace myos {
namespace {
constexpr int iterations=310000;
constexpr size_t salt_size=16, hash_size=32;
std::string hex(const unsigned char* data,size_t count) {
 constexpr char digits[]="0123456789abcdef";
 std::string out;
 for(size_t i=0;i<count;++i) { out+=digits[data[i]>>4]; out+=digits[data[i]&15]; }
 return out;
}
std::vector<unsigned char> unhex(const std::string& s) {
 if(s.size()%2) return {};
 std::vector<unsigned char> out;
 for(size_t i=0;i<s.size();i+=2) {
  auto digit=[](char c)->int {
   if(c>='0'&&c<='9') return c-'0';
   if(c>='a'&&c<='f') return c-'a'+10;
   return -1;
  };
  int a=digit(s[i]),b=digit(s[i+1]);
  if(a<0||b<0) return {};
  out.push_back(static_cast<unsigned char>(a*16+b));
 }
 return out;
}
std::string prompt(const char* label) {
 std::cout<<label<<std::flush;
 std::string s; std::getline(std::cin,s); return s;
}
std::string password_prompt(const char* label) {
 std::cout<<label<<std::flush;
#ifdef _WIN32
 if(_isatty(_fileno(stdin))) {
  std::string s;
  for(;;) {
   int ch=_getch();
   if(ch=='\r'||ch=='\n') break;
   if(ch==8&&!s.empty()) { s.pop_back(); continue; }
   if(ch>=32&&ch<=126) s.push_back(static_cast<char>(ch));
  }
  std::cout<<'\n'; return s;
 }
#else
 termios old{};
 if(isatty(STDIN_FILENO)&&tcgetattr(STDIN_FILENO,&old)==0) {
  termios hidden=old; hidden.c_lflag &= ~ECHO;
  if(tcsetattr(STDIN_FILENO,TCSANOW,&hidden)==0) {
   std::string s; std::getline(std::cin,s);
   tcsetattr(STDIN_FILENO,TCSANOW,&old);
   std::cout<<'\n'; return s;
  }
 }
#endif
 std::string s; std::getline(std::cin,s); return s;
}
struct Record { std::string name,salt,hash; };
std::vector<Record> read_records(const std::filesystem::path& path) {
 std::vector<Record> records;
 std::ifstream file(path);
 if(!file&&std::filesystem::exists(path)) throw std::runtime_error("Cannot read account database");
 std::string line;
 while(std::getline(file,line)) {
  std::istringstream stream(line); Record r;
  if(!(stream>>r.name>>r.salt>>r.hash)) throw std::runtime_error("Malformed account database");
  std::string extra;
  if(stream>>extra) throw std::runtime_error("Malformed account database");
  records.push_back(r);
 }
 return records;
}
std::array<unsigned char,hash_size> derive(const std::string& password,const unsigned char* salt,size_t salt_length) {
 std::array<unsigned char,hash_size> hash{};
 if(!PKCS5_PBKDF2_HMAC(password.data(),static_cast<int>(password.size()),salt,
  static_cast<int>(salt_length),iterations,EVP_sha256(),
  static_cast<int>(hash.size()),hash.data())) throw std::runtime_error("Password hashing failed");
 return hash;
}
bool valid_username(const std::string& s) {
 if(s.empty()||s.size()>32) return false;
 for(unsigned char c:s) if(!std::isalnum(c)&&c!='_'&&c!='-') return false;
 return true;
}
}
Users::Users(std::filesystem::path directory):directory_(std::move(directory)) {
 std::filesystem::create_directories(directory_);
#ifndef _WIN32
 chmod(directory_.string().c_str(),0700);
#endif
}
bool Users::register_user() {
 const auto name=prompt("New XSoft username: ");
 if(!std::cin) return false;
 if(!valid_username(name)) { std::cout<<"Use 1-32 letters, numbers, _ or -.\n"; return false; }
 const auto path=directory_/"users.db";
 for(const auto& r:read_records(path))
  if(r.name==name) { std::cout<<"Username already exists.\n"; return false; }
 const auto password=password_prompt("New password (8+ characters): ");
 if(!std::cin) return false;
 if(password.size()<8||password.size()>1024) { std::cout<<"Password must be 8-1024 characters.\n"; return false; }
 const auto confirm=password_prompt("Confirm password: ");
 if(!std::cin) return false;
 if(password!=confirm) { std::cout<<"Passwords do not match.\n"; return false; }
 std::array<unsigned char,salt_size> salt{};
 if(RAND_bytes(salt.data(),static_cast<int>(salt.size()))!=1)
  throw std::runtime_error("Secure random generation failed");
 const auto hash=derive(password,salt.data(),salt.size());
 std::ofstream file(path,std::ios::app);
 if(!file) throw std::runtime_error("Cannot write account database");
 file<<name<<' '<<hex(salt.data(),salt.size())<<' '<<hex(hash.data(),hash.size())<<'\n';
 file.flush();
 if(!file) throw std::runtime_error("Failed to save account");
#ifndef _WIN32
 chmod(path.string().c_str(),0600);
#endif
 current_=name;
 std::cout<<"XSoft account created. Welcome, "<<current_<<"!\n";
 return true;
}
bool Users::login_user() {
 const auto name=prompt("XSoft username: ");
 if(!std::cin) return false;
 const auto password=password_prompt("Password: ");
 if(!std::cin) return false;
 for(const auto& r:read_records(directory_/"users.db")) {
  if(r.name!=name) continue;
  const auto salt=unhex(r.salt),expected=unhex(r.hash);
  if(salt.size()!=salt_size||expected.size()!=hash_size)
   throw std::runtime_error("Malformed account record");
  const auto hash=derive(password,salt.data(),salt.size());
  if(CRYPTO_memcmp(hash.data(),expected.data(),hash.size())==0) {
   current_=name;
   std::cout<<"Welcome back, "<<current_<<"!\n";
   return true;
  }
  break;
 }
 std::cout<<"Invalid username or password.\n"; return false;
}
bool Users::authenticate() {
 current_.clear();
 while(std::cin) {
  const auto action=prompt("XSoft account [login/register/quit]: ");
  if(!std::cin||action=="quit") return false;
  try {
   if(action=="register"&&register_user()) return true;
   if(action=="login"&&login_user()) return true;
   if(action!="login"&&action!="register")
    std::cout<<"Choose login, register or quit.\n";
  } catch(const std::exception& e) { std::cout<<"Account error: "<<e.what()<<'\n'; }
 }
 return false;
}
}