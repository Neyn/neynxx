#include "server.h"

#include <stdio.h>

#include <thread>

extern "C" {
#include <cneyn/cneyn.h>
}

namespace Neyn
{
void handler(const neyn_request *_request, neyn_response *_response, void *data)
{
    size_t i = 0;
    auto server = static_cast<Server *>(data);

    Request request;
    request.port = _request->port;
    request.major = _request->major;
    request.minor = _request->minor;
    request.method.assign(_request->method.ptr, _request->method.len);
    request.path.assign(_request->path.ptr, _request->path.len);
    request.body.assign(_request->body.ptr, _request->body.len);
    request.address.assign(_request->address);

    std::string name, value;
    for (; i < _request->header.len; ++i)
    {
        neyn_header *header = &_request->header.ptr[i];
        name.assign(header->name.ptr, header->name.len);
        value.assign(header->value.ptr, header->value.len);
        auto &_value = request.header[name];
        _value += _value.empty() ? value : "," + value;
    }

    Response response;
    // clang-format off
    try { server->handler(request, response); }
    catch (Status status) { response.status = status; }
    // clang-format on

    _response->status = neyn_status(response.status);
    _response->file = response.file;
    _response->body.len = response.body.size();
    _response->body.ptr = (char *)response.body.data();

    i = 0;
    neyn_header header[response.header.size()];
    for (const auto &pair : response.header)
    {
        header[i].name.ptr = (char *)pair.first.data();
        header[i].name.len = pair.first.size();
        header[i].value.ptr = (char *)pair.second.data();
        header[i++].value.len = pair.second.size();
    }

    _response->header.ptr = header;
    _response->header.len = response.header.size();
    neyn_response_write(_request, _response);
}

Server::Server(Handler handler, Config config) : data(nullptr), config(std::move(config)), handler(handler) {}

Error Server::run(bool block)
{
    if (config.threads == 0) config.threads = std::thread::hardware_concurrency();
    auto server = new neyn_server;
    data = server;
    server->data = this;
    server->handler = Neyn::handler;

    server->config.ipvn = neyn_address(config.ipvn);
    server->config.port = config.port;
    server->config.address = (char *)config.address.data();
    server->config.timeout = config.timeout;
    server->config.limit = config.limit;
    server->config.threads = config.threads;

    auto error = Error(neyn_server_run(server, block));
    if (block || error != Error::None) delete server;
    return error;
}

void Server::kill()
{
    auto server = static_cast<neyn_server *>(data);
    neyn_server_kill(server);
    delete server;
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
