#include "Server.hpp"
#include "webPages.hpp"

namespace Webserv {

Server::Server(Configuration::server const &config)
    : _socket(ServerSocket(config._listen.second, config._listen.first)), _conf(config)
{
}

Server::~Server()
{
    std::cout << "destructeur called" << std::endl;
}

void Server::handler(int sig)
{
    (void)sig;
    close(Log::fd);
    exit(0);
}

void Server::start (void)
{
    signal(SIGINT, handler);
	signal(SIGQUIT, handler);
    Log::out(_conf._server_name, "Start server");
    main_cycle();
}

int Server::main_cycle(void)
{
    int nbytes;
    char buffer[9000];

    struct sockaddr_in cli_addr;
    unsigned int cli_len = sizeof(cli_addr);
    int cli_sock;

    int max_fd = _socket.get_fd();
    fd_set master_set;
    fd_set tmp_set;
    FD_ZERO(&master_set);
    FD_ZERO(&tmp_set);

    FD_SET(_socket.get_fd(), &master_set);
    while (true) {
        tmp_set = master_set;
        select(max_fd + 1, &tmp_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_fd; ++i) {
            if (FD_ISSET(i, &tmp_set)) {
                if (i == _socket.get_fd()) { // new connection
                    cli_sock = accept(_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&cli_addr), &cli_len);
                    if (cli_sock == -1) {
                        Log::out(_conf._server_name, "accept error");
                        std::cerr << "accept error\n";
                    } else {
                        FD_SET(cli_sock, &master_set);
                        (cli_sock > max_fd) ? max_fd = cli_sock : 0;
                        Log::out(_conf._server_name, "new connection");
                        std::cout << "server: new connection\n";
                    }
                } else {
                    if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
                        if (nbytes == 0) {
                            Log::out(_conf._server_name, "connection dropped");
                            std::cout << "server: connection dropped\n";
                        } else {
                            Log::out(_conf._server_name, "recv error");
                            std::cerr << "recv error\n";
                        }
                        close(i);
                        FD_CLR(i, &master_set);
                    } else {
                        Log::out(_conf._server_name, "reception..");
                        std::cout << "server: received:\n" << buffer << "\n";
                        bzero(buffer, 9000);

                        FD_ZERO(&tmp_set);
                        FD_SET(i, &tmp_set);
                        select(i + 1, NULL, &tmp_set, NULL, NULL);
                        if (!FD_ISSET(cli_sock, &tmp_set)) {
                            Log::out(_conf._server_name, "cannot send data to client");
                            std::cout << "server: cannot send data to client\n";
                            continue ;
                        }

                        /*	Send name specified by GET request to the object
                            (index.hmtl will be loaded if no file is specified
                            404.html will be loaded if the requested file can't be opened	*/
                        webPages	page;
                        page.setHeader();
                        page.setPages("index.html");
                        send(i, page.getPages(), page.getLength(), 0);
                    }
                }
            }
        }
    }
    return 0;
}

}; // namespace Webserv
