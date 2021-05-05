#include "Logger.hpp"

namespace Webserv {
namespace Log {

void prepare_file()
{
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH;
    if ((fd = open("log", O_CREAT | O_RDWR | O_TRUNC, mode)) == -1)
        std::cerr << "Can't create log" << std::endl;
}

void out(std::string const &server_name, std::string const &str)
{
    if (fd == -1)
        return ;
    std::string res (Time::get_date_logger_format());
    if (!server_name.empty())
        res += ("[" + server_name + "] ");
    int width = res.length() + 1;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (*it != '\r')
            res.push_back(*it);
    }
    for (size_t i = 1; i < res.length() - 1; ++i) {
        if (res[i - 1] == '\n') {
            res.insert(i, width, ' ');
            i += width;
        }
    }
    if (str[str.length() - 1] != '\n')
        res += "\n";
    write(fd, res.c_str(), res.length());
}

};
};
