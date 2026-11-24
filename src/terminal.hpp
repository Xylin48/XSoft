#pragma once
#include <iostream>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
namespace myos {
class Terminal {
 bool active=false;
#ifdef _WIN32
 HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE);
 DWORD previous=0;
#endif
public:
 Terminal(){
#ifdef _WIN32
 active=GetConsoleMode(handle,&previous) && SetConsoleMode(handle,previous|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
 active=isatty(STDOUT_FILENO);
#endif
 if(active) std::cout<<"\033[0;92;40m\033[2J\033[H"<<std::flush;
 }
 ~Terminal(){
 if(active) std::cout<<"\033[0m"<<std::flush;
#ifdef _WIN32
 if(active) SetConsoleMode(handle,previous);
#endif
 }
 Terminal(const Terminal&)=delete;
 Terminal& operator=(const Terminal&)=delete;
};
}
