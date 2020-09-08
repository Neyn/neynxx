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
    _response->fsize = response.fsize;
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

Server::Server(Config config, Handler handler) : config(std::move(config)), handler(handler), data(nullptr) {}

void init(Server *dis, neyn_server *server)
{
    server->data = dis;
    server->handler = Neyn::handler;
    server->config.ipvn = neyn_address(dis->config.ipvn);
    server->config.port = dis->config.port;
    server->config.address = (char *)dis->config.address.data();
    server->config.timeout = dis->config.timeout;
    server->config.limit = dis->config.limit;
    server->config.threads = dis->config.threads;
}

Error Server::run(bool block)
{
    if (config.threads == 0) config.threads = std::thread::hardware_concurrency();
    auto server = new neyn_server;
    data = server;
    init(this, server);

    auto error = Error(neyn_server_run(server, block));
    if (block || error != Error::None) delete server;
    return error;
}

Error Server::single()
{
    neyn_server server;
    init(this, &server);
    return Error(neyn_single_run(&server));
}

void Server::kill()
{
    auto server = static_cast<neyn_server *>(data);
    neyn_server_kill(server);
    delete server;
}

bool Response::open(const std::string &path)
{
    neyn_size size;
    if (file != nullptr) fclose(file);
    file = neyn_file_open(path.c_str(), &size);
    if (file != nullptr) fsize = size;
    return file != nullptr;
}
}  // namespace Neyn
