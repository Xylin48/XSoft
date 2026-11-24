#pragma once
#include <filesystem>
#include <string>
namespace myos {
class Drive {
public:
 explicit Drive(std::filesystem::path root);
 void reset();
 std::string cwd_display() const;
 const std::filesystem::path& root() const { return root_; }
 bool command(const std::string& command);
private:
 std::filesystem::path root_;
 std::filesystem::path cwd_; // relative to C:, never a host path
 std::filesystem::path resolve(const std::string& input) const;
};
}