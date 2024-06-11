#include "http_TcpServer.h"

using namespace http;

int main(void)
{
    int const PORT = 8080;
    TcpServer server = TcpServer("0.0.0.0", PORT);

    server.startListen();
    return 0;
}
