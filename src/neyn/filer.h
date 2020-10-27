#pragma once

#include <neyn/server.h>

#include <iostream>
#include <string>

namespace Neyn
{
struct Filer
{
    using Handler = std::function<void(Request &, Response &)>;

    Filer(const std::string &base = {}, const std::string &root = {}, Handler error = {});
    void operator()(Request &request, Response &response);

private:
    Handler error;
    std::string base, root;

    friend std::ostream &operator<<(std::ostream &os, const Filer &filer);
};
}  // namespace Neyn
