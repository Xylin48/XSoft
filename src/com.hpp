#pragma once
namespace myos {
class Users;
class Drive;
enum class ShellResult { Shutdown, Reboot };
ShellResult run_shell(Users& users,Drive& drive);
}