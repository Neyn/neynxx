#include <iostream>

#include "neynxx/neynxx.h"

using namespace std;
using namespace Neynxx;

void handler(Request &request, Response &response) { response.body = "Hello"; }

int main()
{
    Server server;
    server.config.threads = 1;
    server.handler = handler;
    server.run();
    return 0;
}
