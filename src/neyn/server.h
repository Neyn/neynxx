#pragma once

#include <neyn/config.h>

#include <functional>
#include <iostream>
#include <map>
#include <string>

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

enum class Address
{
    IPV4,
    IPV6
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
    FILE *file = NULL;
    std::map<std::string, std::string> header;
};

struct Config
{
    uint16_t port = 8081;
    Address ipvn = Address::IPV4;
    std::string address = "0.0.0.0";
    size_t timeout = 0, limit = 0, threads = 1;
};

struct Server
{
    using Handler = std::function<void(Request &, Response &)>;

    void *data;
    Config config;
    Handler handler;

    Server(Handler handler = {}, Config config = {});
    Error run(bool block = true);
    Error single();
    void kill();
};

std::ostream &operator<<(std::ostream &os, const Config &config);
std::ostream &operator<<(std::ostream &os, const Request &request);
std::ostream &operator<<(std::ostream &os, const Response &response);
}  // namespace Neyn
