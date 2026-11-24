#pragma once
namespace myos {
enum class ShellResult { Shutdown, Reboot };
ShellResult run_shell();
}
