#include "Server.hpp"

namespace Webserv {

Server::Server(void)
    : _socket(ServerSocket("127.0.0.1", 8080)
{
}

void Server::start (void)
{
    main_cycle();
}

int Server::main_cycle(void)
{
    int nbytes;
    char buffer[256];

    struct sockaddr_in cli_addr;
    unsigned int cli_len = sizeof(cli_addr);
    int cli_sock;

    int max_fd = m_server_sock;
    fd_set master_set;
    fd_set tmp_set;
    FD_ZERO(&master_set);
    FD_ZERO(&tmp_set);

    FD_SET(m_server_sock, &master_set);

    while (true) {
        FD_COPY(&master_set, &tmp_set);
        select(max_fd + 1, &tmp_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_fd; ++i) {
            if (FD_ISSET(i, &tmp_set)) {
                if (i == m_server_sock) { // new connection
                    cli_sock = accept(m_server_sock, reinterpret_cast<struct sockaddr*>(&cli_addr), &cli_len);
                    if (cli_sock == -1) {
                        std::cerr << "accept error\n";
                    } else {
                        FD_SET(cli_sock, &master_set);
                        (cli_sock > max_fd) ? max_fd = cli_sock : 0;
                        std::cout << "server: new connection\n";
                    }
                } else {
                    if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
                        if (nbytes == 0) {
                            std::cout << "server: connection dropped\n";
                        } else {
                            std::cerr << "recv error\n";
                        }
                        close(i);
                        FD_CLR(i, &master_set);
                    } else {
                        std::cout << "server: received:\n" << buffer << "\n";
                        bzero(buffer, 256);

                        FD_ZERO(&tmp_set);
                        FD_SET(i, &tmp_set);
                        select(i + 1, NULL, &tmp_set, NULL, NULL);
                        if (!FD_ISSET(cli_sock, &tmp_set)) {
                            std::cout << "server: cannot send data to client\n";
                            continue ;
                        }
                        std::string s = "this is some random data\n";
                        write(i, s.c_str(), s.length());
                    }
                }
            }
        }
    }
    return 0;
}

}; // namespace Webserv
