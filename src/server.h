#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "config.h"

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

enum class Error
{
    None,
    GeneralError,
    SocketError,
    SocketCreate,
    SocketListen,
    SocketAccept,
    SetReuse,
    SetNonblock,
    SetAddress,
    EpollCreate,
    EpollAdd,
    EpollWait,
    TimerCreate,
    ThreadCreate,
    WrongParameter,
};

enum class Status
{
    Continue,
    SwitchingProtocols,
    Processing,
    OK,
    Created,
    Accepted,
    NonAuthoritativeInformation,
    NoContent,
    ResetContent,
    PartialContent,
    MultiStatus,
    AlreadyReported,
    IMUsed,
    MultipleChoices,
    MovedPermanently,
    Found,
    SeeOther,
    NotModified,
    UseProxy,
    TemporaryRedirect,
    PermanentRedirect,
    BadRequest,
    Unauthorized,
    PaymentRequired,
    Forbidden,
    NotFound,
    MethodNotAllowed,
    NotAcceptable,
    ProxyAuthenticationRequired,
    RequestTimeout,
    Conflict,
    Gone,
    LengthRequired,
    PreconditionFailed,
    PayloadTooLarge,
    RequestURITooLong,
    UnsupportedMediaType,
    RequestedRangeNotSatisfiable,
    ExpectationFailed,
    ImATeapot,
    MisdirectedRequest,
    UnprocessableEntity,
    Locked,
    FailedDependency,
    UpgradeRequired,
    PreconditionRequired,
    TooManyRequests,
    RequestHeaderFieldsTooLarge,
    ConnectionClosedWithoutResponse,
    UnavailableForLegalReasons,
    ClientClosedRequest,
    InternalServerError,
    NotImplemented,
    BadGateway,
    ServiceUnavailable,
    GatewayTimeout,
    HTTPVersionNotSupported,
    VariantAlsoNegotiates,
    InsufficientStorage,
    LoopDetected,
    NotExtended,
    NetworkAuthenticationRequired,
    NetworkConnectTimeoutError,
};

struct Request
{
    uint16_t port, major, minor;
    std::string address, method, path, body;
    std::map<std::string, std::string> header;
};

struct Response
{
    Status status = Status::OK;
    std::string body;
    std::map<std::string, std::string> header;
};

struct Config
{
    uint16_t port;
    std::string address;
    size_t timeout, limit, threads;
    inline Config() : port(8080), address("0.0.0.0"), timeout(0), limit(0), threads(0) {}
    Config(uint16_t port, std::string address, size_t timeout, size_t limit, size_t threads);
};

struct Server
{
    using Handler = std::function<void(Request &, Response &)>;

    Config config;
    Handler handler;

    Server(Handler handler = {}, Config config = {});
    Error run();
};

std::ostream &operator<<(std::ostream &os, const Config &config);
std::ostream &operator<<(std::ostream &os, const Request &request);
std::ostream &operator<<(std::ostream &os, const Response &response);
}  // namespace Neyn
