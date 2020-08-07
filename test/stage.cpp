#include <iostream>

#include "neyn/neyn.h"

using namespace std;
using namespace Neyn;

void handler(Request &request, Response &response) { response.body = "Hello"; }

int main()
{
    Server server;
    server.config.threads = 1;
    server.handler = handler;
    server.run();
    return 0;
}
