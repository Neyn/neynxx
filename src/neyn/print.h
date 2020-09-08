#pragma once

#include <neyn/filer.h>
#include <neyn/server.h>

#include <iostream>

namespace Neyn
{
template <typename T>
void print(const T &value)
{
    std::cout << value << " " << std::endl;
}
template <typename Head, typename... Tail>
void print(const Head &head, Tail &&... tail)
{
    std::cout << head << " ";
    print(std::forward<Tail>(tail)...);
}
inline void print() { std::cout << std::endl; }

std::ostream &operator<<(std::ostream &os, const Filer &filer);
std::ostream &operator<<(std::ostream &os, const Config &config);
std::ostream &operator<<(std::ostream &os, const Request &request);
std::ostream &operator<<(std::ostream &os, const Response &response);
}  // namespace Neyn
