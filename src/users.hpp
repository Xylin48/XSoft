#pragma once
#include <filesystem>
#include <string>
namespace myos {
class Users {
public:
 explicit Users(std::filesystem::path directory);
 bool authenticate();
 void logout() { current_.clear(); }
 const std::string& current() const { return current_; }
private:
 std::filesystem::path directory_;
 std::string current_;
 bool register_user();
 bool login_user();
};
}