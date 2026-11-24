#pragma once
namespace myos {
class Users;
enum class ShellResult { Shutdown, Reboot };
ShellResult run_shell(Users& users);
}