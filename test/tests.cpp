#include <arpa/inet.h>
#include <neyn/neyn.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#define CLEAR "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define SEPARATOR "--------------------" << endl

#define CHECK(expr)                                                                                          \
    if (!static_cast<bool>(expr))                                                                            \
    {                                                                                                        \
        cout << RED << "\"" << #expr << "\" Failed at Line " << to_string(__LINE__) << "!" << CLEAR << endl; \
        throw 0;                                                                                             \
    }

#define TEST(name)                                                                      \
    void name##Test_();                                                                 \
    void name##Test()                                                                   \
    {                                                                                   \
        H::Current = #name;                                                             \
        cout << #name << " Test Started..." << endl;                                    \
        try                                                                             \
        {                                                                               \
            name##Test_();                                                              \
            cout << GREEN << #name << " Test succeeded!" << CLEAR << endl << SEPARATOR; \
        }                                                                               \
        catch (...)                                                                     \
        {                                                                               \
            cout << RED << #name << " Test Failed!" << CLEAR << endl << SEPARATOR;      \
            if (H::Code != 0) H::Code = 1;                                              \
        }                                                                               \
    }                                                                                   \
    void name##Test_()

#define PROCESS(input)          \
    auto fd = H::create();      \
    CHECK(fd >= 0);             \
    CHECK(H::write(fd, input)); \
    auto data = H::read(fd);    \
    auto info = H::parse(data);

using namespace std;

namespace H
{
int Code = 0;
string Current;

struct Info
{
    size_t code;
    string phrase, body;
    map<string, string> header;
};

string read(int fd)
{
    // TODO change to when socket closes.
    string data, temp(1000, 0);
    while (data.find("\r\n\r\n") == string::npos)
    {
        auto bytes = ::read(fd, &temp[0], temp.size());
        if (bytes < 0) return string();
        data.append(temp.substr(0, bytes));
    }

    auto prev = data.size(), head = data.find("\r\n\r\n") + 4;
    auto it = data.find("Content-Length") + string("Content-Length").size() + 2;
    auto body = strtoul(data.data() + it, NULL, 10);

    data.resize(head + body);
    for (size_t i = prev; i < head + body;)
    {
        auto bytes = ::read(fd, &data[0] + i, head + body - i);
        if (bytes < 0) return string();
        i += bytes;
    }
    return data;
}

bool write(int fd, const string &data)
{
    for (size_t i = 0; i < data.size(); ++i)
    {
        auto result = ::write(fd, data.data() + i, 1);
        if (result != 1) return false;
        usleep(1000);
    }
    return true;
}

bool fwrite(int fd, const string &data)
{
    for (size_t i = 0, tries = 0; i < data.size();)
    {
        if (++tries > 1000) return false;
        auto bytes = ::write(fd, data.data() + i, data.size() - i);
        if (bytes < 0) return false;
        i += bytes;
    }
    return true;
}

Info parse(const string &data)
{
    Info info;
    size_t prev = 0, curr;
    curr = data.find("\r\n", prev);
    string name, value, line = data.substr(prev, curr - prev);

    curr = data.find(' ', prev), prev = curr + 1, curr = data.find(' ', prev);
    info.code = stoul(line.substr(prev, curr - prev)), prev = curr + 1;
    info.phrase = line.substr(prev), prev = line.size() + 2;

    while (true)
    {
        curr = data.find("\r\n", prev);
        line = data.substr(prev, curr - prev);
        if (line.empty()) break;

        curr = line.find(':');
        name = line.substr(0, curr), value = line.substr(curr + 2);
        info.header.insert({name, value}), prev += line.size() + 2;
    }

    info.body = data.substr(curr + 2);
    return info;
}

int create()
{
    auto port = 8081;
    auto ip = "127.0.0.1";

    struct sockaddr_in address;
    address.sin_family = AF_INET, address.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &address.sin_addr) <= 0) return -1;

    int socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) return socket;
    return connect(socket, (struct sockaddr *)&address, sizeof(address)) < 0 ? -1 : socket;
}
}  // namespace H

TEST(Syntax)
{
    {
        PROCESS("GET\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("/\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("HTTP/1.1\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("/ HTTP/1.1\r\n\r\n");
        CHECK(info.code == 501) CHECK(info.body.empty()) CHECK(info.phrase == "Not Implemented");
    }
    {
        PROCESS("GET HTTP/1.1\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("GET /\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("CRAP / HTTP/1.1\r\n\r\n");
        CHECK(info.code == 501) CHECK(info.body.empty()) CHECK(info.phrase == "Not Implemented");
    }
    {
        PROCESS("\r \nGET / HTTP/1.1\r\n\r\n");
        CHECK(info.code == 501) CHECK(info.body.empty()) CHECK(info.phrase == "Not Implemented");
    }
    {
        PROCESS("\r\n \r\nGET / HTTP/1.1\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("GET / HTTP/1.1\r\nA :B\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("\r\n\r\n\r\n\r\n\r\nGET / HTTP/1.1\r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body.empty()) CHECK(info.phrase == "OK")
    }
    {
        PROCESS("   \t \t GET  \t\t   /pathpathpath  \t   \t HTTP/1.1 \t  \t     \t   \r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body.empty()) CHECK(info.phrase == "OK")
    }
    {
        PROCESS("GET / HTTP/1.1\r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body.empty()) CHECK(info.phrase == "OK")
    }
    {
        PROCESS("\r\nGET / HTTP/1.1\r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body.empty()) CHECK(info.phrase == "OK")
    }
    {
        PROCESS("GET / HTTP/1.1111\r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body.empty()) CHECK(info.phrase == "OK")
    }
    {
        PROCESS("GET / HTTP/1.11111\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
    {
        PROCESS("GET / HTTP/2.1\r\n\r\n");
        CHECK(info.code == 505) CHECK(info.body.empty()) CHECK(info.phrase == "HTTP Version Not Supported");
    }
    {
        PROCESS("GET / HTTP/11111.1\r\n\r\n");
        CHECK(info.code == 400) CHECK(info.body.empty()) CHECK(info.phrase == "Bad Request");
    }
}

TEST(Get)
{
    {
        PROCESS("GET / HTTP/1.1\r\n A:B \r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body == "Hello") CHECK(info.phrase == "OK");
        CHECK(info.header["A"] == "B") CHECK(info.header["major"] == "1") CHECK(info.header["minor"] == "1");
    }
    {
        PROCESS("GET / HTTP/1.1\r\n A:B \r\n C:D \r\n\r\n")
        CHECK(info.code == 200) CHECK(info.body == "Hello") CHECK(info.phrase == "OK");
        CHECK(info.header["A"] == "B") CHECK(info.header["major"] == "1") CHECK(info.header["minor"] == "1");
    }
}

TEST(Post)
{
    {
        PROCESS("POST / HTTP/1.1 \r\n A:B \r\n C:D \r\n content-length:5 \r\n\r\nHello")
        CHECK(info.code == 200) CHECK(info.body == "Hello") CHECK(info.phrase == "OK");
        CHECK(info.header["A"] == "B") CHECK(info.header["C"] == "D");
    }
}

TEST(Chunk)
{
    {
        PROCESS("POST / HTTP/1.1 \r\n Transfer-Encoding:chunked \r\n\r\n5\r\n12345\r\n3\r\n123\r\n1\r\n1\r\n0\r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body == "123451231") CHECK(info.phrase == "OK");
    }
}

void handler(Neyn::Request &request, Neyn::Response &response)
{
    if (H::Current == "Get")
    {
        response.body = "Hello";
        response.header = request.header;
        response.header["major"] = to_string(request.major);
        response.header["minor"] = to_string(request.minor);
    }
    else
    {
        response.body = request.body;
        response.header = request.header;
    }
}

int main()
{
    Neyn::Server server;
    server.handler = handler;
    auto error = server.run(false);
    usleep(1000);

    if (error != Neyn::Error::None)
    {
        cout << RED << "Tests Failed!" << CLEAR << endl;
        return 1;
    }

    cout << SEPARATOR << "Running Tests..." << endl << SEPARATOR;
    SyntaxTest();
    GetTest();
    PostTest();
    ChunkTest();
    cout << "Tests Done!" << endl << SEPARATOR;

    usleep(1000);
    server.kill();
    return H::Code;
}
