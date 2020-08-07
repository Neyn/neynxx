#include "server.h"

#include <thread>

extern "C" {
#include "cneyn/cneyn.h"
}

namespace Neyn
{
void handler(neyn_request *_request, neyn_output *output, void *data)
{
    size_t i = 0;
    auto &server = *static_cast<Server *>(data);

    Request request;
    request.port = _request->port;
    request.major = _request->major;
    request.minor = _request->minor;
    request.method.assign(_request->method_ptr, _request->method_len);
    request.path.assign(_request->path_ptr, _request->path_len);
    request.body.assign(_request->body_ptr, _request->body_len);
    request.address.assign(_request->address);

    std::string name, value;
    for (; i < _request->header_len; ++i)
    {
        neyn_header *header = &_request->header_ptr[i];
        name.assign(header->name_ptr, header->name_len);
        value.assign(header->value_ptr, header->value_len);
        auto &_value = request.header[name];
        _value += _value.empty() ? value : "," + value;
    }

    Response response;
    // clang-format off
    try { server.handler(request, response); }
    catch (Status status) { response.status = status; }
    // clang-format on

    neyn_response _response;
    neyn_response_init(&_response);
    _response.status = neyn_status(response.status);
    _response.body_ptr = (char *)response.body.data();
    _response.body_len = response.body.size();
    if (!response.body.empty()) response.header["Content-Length"] = std::to_string(response.body.size());

    i = 0;
    neyn_header header[response.header.size()];
    for (const auto &pair : response.header)
    {
        header[i].name_ptr = (char *)pair.first.data();
        header[i].name_len = pair.first.size();
        header[i].value_ptr = (char *)pair.second.data();
        header[i++].value_len = pair.second.size();
    }

    _response.header_ptr = header;
    _response.header_len = response.header.size();
    neyn_response_write(&_response, output);
}

Config::Config(uint16_t port, std::string address, size_t timeout, size_t limit, size_t threads)
    : port(port), address(address), timeout(timeout), limit(limit), threads(threads)
{
}

Server::Server(Handler handler, Config config) : config(std::move(config)), handler(handler) {}

Error Server::run()
{
    if (config.threads == 0) config.threads = std::thread::hardware_concurrency();

    neyn_server server;
    server.data = this;
    server.port = config.port;
    server.address = (char *)config.address.c_str();
    server.timeout = config.timeout;
    server.limit = config.limit;
    server.handler = Neyn::handler;
    server.threads = config.threads;
    return Error(neyn_server_run(&server));
}

std::string ws = "    ";

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
