#include <neyn/neyn.h>

#include <iostream>

using namespace std;
using namespace Neyn;

void handler(Request &request, Response &response) { cout << request << endl; }

int main()
{
    Server server;
    server.handler = handler;
    auto error = server.run();
    cout << int(error) << endl;
    return 0;
}
