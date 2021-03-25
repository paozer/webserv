#include "Server.hpp"


#include "webPages.hpp"

namespace Webserv {

Server::Server(Configuration::server const &config)
    : _conf(config), _socket(ServerSocket(config._listen.second, config._listen.first))
{
}

void Server::start (void)
{
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

    // socket(AF_INET, SOCK_STREAM, 0));
    // (fcntl)(_socket.get_fd(), F_SETFL, O_NONBLOCK);


    while (true) {
        tmp_set = master_set;
        select(max_fd + 1, &tmp_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_fd; ++i) {
            if (FD_ISSET(i, &tmp_set)) {
                if (i == _socket.get_fd()) { // new connection
                    cli_sock = accept(_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&cli_addr), &cli_len);
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
                        bzero(buffer, 9000);

                        FD_ZERO(&tmp_set);
                        FD_SET(i, &tmp_set);
                        select(i + 1, NULL, &tmp_set, NULL, NULL);
                        if (!FD_ISSET(cli_sock, &tmp_set)) {
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

			// FD_CLR(i, &master_set);
            //             close(i);
                }
            }
            }
        }
    }
    return 0;
}

/*#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

std::string	Server::get_html_pages(const std::string& name) {
	std::string 	ret;
	std::string 	tmp;
	std::string 	t;
	int		file = 0;

	ret += "HTTP/1.1 200 OK\n";
	ret += "Server: webserv\n";
	ret += "Content-Type: text/html\n";
	ret += "Connection: close\n\n";
	t = "./www/" + name;
	if ((file = open(t.c_str(), O_RDONLY)) < 0)
		return ("NULL");
	int	rd = 0;
	char *buffer = (char*)malloc(50);
	while ((rd = read(file, buffer, 49)) > 0) {
		buffer[rd] = '\0';
		ret += buffer;
	}
	free(buffer);
	close(file);
	return (ret);
}*/

/*std::string	Server::get_html_pages(const std::string& name) {
	std::string ret;
	std::string tmp;
	std::string t;
	std::ifstream input_files;

	ret += "HTTP/1.1 200 OK\n";
	ret += "Server: webserv\n";
	ret += "Content-Type: text/html\n";
	ret += "Connection: close\n\n";
	t = "./www/" + name;
	std::cout << t.c_str() << std::endl;
	input_files.open(t.c_str());
	if (input_files.is_open()) {
		std::cout << "file is open" << std::endl;
		while (getline(input_files, tmp)) {
			ret += tmp;
			ret += "\n";
		}
	}
	else
		ret = "404 not found";
	input_files.close();
	return (ret);
}*/

}; // namespace Webserv
