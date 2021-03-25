#pragma once

#include "ServerSocket.hpp"
#include "Parsing.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h> // close, unlink
#include <fcntl.h> // FD_MACROS
#include <strings.h> // bzero
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

namespace Webserv {

class Server
{
    public:
        Server(Configuration::server const &config);
        void start (void);
        void stop (void);

    private:
        Server();
        ServerSocket    _socket;
        Configuration::server   _conf;
        int main_cycle(void);
}; // class Server

}; // namespace Webserv
