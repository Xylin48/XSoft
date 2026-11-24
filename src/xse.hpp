#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <openssl/evp.h>
namespace myos::xse {
using Bytes = std::vector<unsigned char>;
inline constexpr std::size_t limit = 64u * 1024u * 1024u;
inline void put(Bytes& b, std::uint32_t n) {
 for(unsigned i=0;i<4;++i) b.push_back(static_cast<unsigned char>(n>>(8*i)));
}
inline Bytes digest(const Bytes& b) {
 Bytes d(32); unsigned n=0;
 if(EVP_Digest(b.data(),b.size(),d.data(),&n,EVP_sha256(),nullptr)!=1 || n!=32)
  throw std::runtime_error("XSE digest failed");
 return d;
}
inline bool valid_id(const std::string& s) {
 return !s.empty() && s.size()<=64 && std::all_of(s.begin(),s.end(),[](unsigned char c){
 return (c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'; });
}
struct Package { std::string id; Bytes payload; };
inline Bytes encode(const Package& p) {
 if(!valid_id(p.id)||p.payload.empty()||p.payload.size()>limit)
  throw std::runtime_error("Invalid XSE package");
 Bytes b{'X','S','E',1};
 put(b,static_cast<std::uint32_t>(p.id.size()));
 put(b,static_cast<std::uint32_t>(p.payload.size()));
 b.insert(b.end(),p.id.begin(),p.id.end());
 b.insert(b.end(),p.payload.begin(),p.payload.end());
 auto d=digest(b); b.insert(b.end(),d.begin(),d.end()); return b;
}
inline Package decode(const Bytes& b) {
 if(b.size()<45 || b.size()>limit+108 || b[0]!='X'||b[1]!='S'||b[2]!='E'||b[3]!=1)
  throw std::runtime_error("Invalid XSE header");
 auto get=[&](std::size_t off){std::uint32_t n=0; for(unsigned i=0;i<4;++i)
 n|=static_cast<std::uint32_t>(b[off+i])<<(8*i); return n;};
 auto id=get(4), size=get(8);
 if(id==0||id>64||size==0||size>limit||b.size()!=12ull+id+size+32)
  throw std::runtime_error("Invalid XSE lengths");
 Bytes body(b.begin(),b.end()-32); auto d=digest(body);
 if(!std::equal(d.begin(),d.end(),b.end()-32)) throw std::runtime_error("XSE checksum mismatch");
 Package p{std::string(b.begin()+12,b.begin()+12+id),Bytes(b.begin()+12+id,b.end()-32)};
 if(!valid_id(p.id)) throw std::runtime_error("Invalid XSE identity");
 return p;
}
}
