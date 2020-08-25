#include <arpa/inet.h>
#include <neyn/neyn.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#define CHECK(expr)                                                                                \
    if (!static_cast<bool>(expr))                                                                  \
    {                                                                                              \
        cout << string("\033[35m") + "<" + #expr + "> Failed at Line " + to_string(__LINE__) + "." \
             << "\033[0m" << endl;                                                                 \
        throw 0;                                                                                   \
    }

#define PROCESS(input)          \
    auto fd = H::create();      \
    CHECK(fd >= 0);             \
    CHECK(H::write(fd, input)); \
    auto data = H::read(fd);    \
    auto info = H::parse(data);

using namespace std;

namespace H
{
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

#define TEST(name)                                                  \
    void name##Test_();                                             \
    void name##Test()                                               \
    {                                                               \
        H::Current = #name;                                         \
        cout << string("\033[33m") + #name + " Test Started..."     \
             << "\033[0m" << endl;                                  \
        try                                                         \
        {                                                           \
            name##Test_();                                          \
            cout << string("\033[32m") + #name + " Test succeeded!" \
                 << "\033[0m" << endl                               \
                 << endl;                                           \
        }                                                           \
        catch (...)                                                 \
        {                                                           \
            cout << string("\033[31m") + #name + " Test Failed!"    \
                 << "\033[0m" << endl                               \
                 << endl;                                           \
        }                                                           \
    }                                                               \
    void name##Test_()

TEST(Simple)
{
    // TODO test CRLF at start
    {
        PROCESS("GET / HTTP/1.1\r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body.empty()) CHECK(info.phrase == "OK")
    }
    {
        PROCESS("   \t \t GET  \t\t   /pathpathpath  \t   \t HTTP/1.1 \t  \t     \t   \r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body.empty()) CHECK(info.phrase == "OK")
    }
}

TEST(Get)
{
    {
        PROCESS("GET / HTTP/1.1\r\n A:B \r\n\r\n");
        CHECK(info.code == 200) CHECK(info.body == "Hello") CHECK(info.phrase == "OK");
        CHECK(info.header["A"] == "B");
    }
    {
        PROCESS("GET / HTTP/1.1\r\n A:B \r\n C:D \r\n\r\n")
        CHECK(info.code == 200) CHECK(info.body == "Hello") CHECK(info.phrase == "OK");
        CHECK(info.header["A"] == "B") CHECK(info.header["C"] == "D");
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
        cout << "Tests Failed!" << endl;
        return 1;
    }

    cout << "\033[36mRunning Tests...\033[0m" << endl << endl;
    SimpleTest();
    GetTest();
    PostTest();
    ChunkTest();
    cout << "\033[36mTests Done!\033[0m" << endl;

    usleep(1000);
    server.kill();
    return 0;
}
