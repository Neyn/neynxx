#include "filer.h"

namespace Neyn
{
Filer::Filer(const std::string &base, const std::string &root) : base(base), root(root)
{
    notfound = [](Request &, Response &response) { response.status = Status::NotFound; };
    notpath = [](Request &, Response &response) { response.status = Status::BadRequest; };
}

void Filer::operator()(Request &request, Response &response)
{
    auto path = request.path;
    if (path.size() < base.size() || !std::equal(base.begin(), base.end(), path.begin()))
        return notpath(request, response);

    path = root + path.substr(base.size());
    if (!response.open(path)) return notfound(request, response);

    // TODO set headers
}
}  // namespace Neyn
