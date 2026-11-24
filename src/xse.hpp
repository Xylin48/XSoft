#pragma once
#include <string>
#include <algorithm>
namespace myos::xse {
inline bool valid_id(const std::string& s) {
 return !s.empty() && s.size()<=64 && std::all_of(s.begin(),s.end(),[](unsigned char c){
 return (c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='-'; });
}
}
