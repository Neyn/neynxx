#include <neyn/neyn.h>

#include <iostream>

using namespace std;
using namespace Neyn;

void handler(Request &request, Response &response) { response.body = "Hello"; }

int main()
{
    Config config;
    config.port = 8081;
    config.ipvn = Address::IPV4;
    config.timeout = 0;
    config.limit = 0;
    config.threads = 1;
    config.address = "0.0.0.0";

    Filer filer("/", "/home/shahriar/Downloads/");
    Server server(config, handler);
    auto error = server.run();
    cout << int(error) << endl;
    return 0;
}
