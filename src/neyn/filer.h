#pragma once

#include <neyn/server.h>

#include <iostream>
#include <string>

namespace Neyn
{
struct Filer
{
    using Handler = std::function<void(Request &, Response &)>;

    std::string base, root;
    Handler error;

    Filer(const std::string &base = {}, const std::string &root = {});
    void operator()(Request &request, Response &response);
};
}  // namespace Neyn
