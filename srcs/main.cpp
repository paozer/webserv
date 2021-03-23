#include "Server.hpp"

int main(int ac, char *av[], char *ev[])
{
    Webserv::Server server;

    server.start();
    return 0;
}
