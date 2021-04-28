#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <iostream>
#include <algorithm>
#include <string>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "Time.hpp"
#include "Utils.hpp"

namespace Webserv {
namespace Log {

static int fd = -1;

void            prepare_file();
void            out(std::string const &server_name, std::string const &str);

};
};

#endif
