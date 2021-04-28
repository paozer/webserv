#ifndef AUTHENTICATION_HPP
#define AUTHENTICATION_HPP

#include "../Server/Methods.hpp"

namespace Webserv {
namespace Http {

bool credentials_are_valid (const std::string& auth_header_value, const std::list<std::string>& credentials);
std::string base64_decode(const std::string& s);

};
};

#endif
