#include "filer.h"

namespace Neyn
{
Filer::Filer(const std::string &base, const std::string &root) : base(base), root(root)
{
    error = [](Request &, Response &response) { response.status = Status::NotFound; };
}

void Filer::operator()(Request &request, Response &response)
{
    auto path = request.path;
    if (path.size() < base.size() || !std::equal(base.begin(), base.end(), path.begin()))
        return error(request, response);

    path = root + path.substr(base.size());
    std::cout << "Path: " << request.path << ", File: " << path << std::endl;
    if (!response.open(path)) return error(request, response);

    // TODO set headers
}
}  // namespace Neyn
