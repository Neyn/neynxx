#include <neyn/neyn.h>

#include <iostream>

using namespace std;
using namespace Neyn;

void handler(Request &request, Response &response) { response.body = "Hello"; }

int main()
{
    Server server;
    Filer filer("/", "/home/shahriar/");
    server.handler = filer;
    auto error = server.run();
    cout << int(error) << endl;
    return 0;
}
