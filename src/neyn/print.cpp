#include "print.h"

extern "C" {
#include <cneyn/cneyn.h>
}

namespace Neyn
{
std::string ws = "    ";

std::ostream &operator<<(std::ostream &os, const Filer &filer)
{
    os << "Filer:" << std::endl;
    os << ws << "Base -> " << filer.base << std::endl;
    os << ws << "Root -> " << filer.root << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Config &config)
{
    os << "Config:" << std::endl;
    os << ws << "Port -> " << config.port << std::endl;
    os << ws << "Address -> " << config.address << std::endl;
    os << ws << "Timeout -> " << config.timeout << std::endl;
    os << ws << "Limit -> " << config.limit << std::endl;
    os << ws << "Threads -> " << config.threads << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Request &request)
{
    os << "Request:" << std::endl;
    os << ws << "Address -> " << request.address << std::endl;
    os << ws << "Port -> " << request.port << std::endl;
    os << ws << "Major -> " << request.major << std::endl;
    os << ws << "Minor -> " << request.minor << std::endl;
    os << ws << "Method -> " << request.method << std::endl;
    os << ws << "Path -> " << request.path << std::endl;
    os << ws << "Headers:" << std::endl;
    for (const auto &pair : request.header) os << ws << ws << pair.first << " -> " << pair.second << std::endl;
    if (!request.body.empty()) os << ws << "Body -> " << request.body << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Response &response)
{
    os << "Response:" << std::endl;
    os << ws << "Status -> " << neyn_status_code[int(response.status)] << std::endl;
    os << ws << "Headers:" << std::endl;
    for (const auto &pair : response.header) os << ws << ws << pair.first << " -> " << pair.second << std::endl;
    if (!response.body.empty()) os << ws << "Body -> " << response.body << std::endl;
    return os;
}

}  // namespace Neyn
