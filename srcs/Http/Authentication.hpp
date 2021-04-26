#ifndef AUTHENTICATION_HPP
#define AUTHENTICATION_HPP

#include "../Server/Methods.hpp"

namespace Webserv {
namespace Http {
std::list<std::string> load_credentials_from_file (const std::string& filepath);
bool credentials_are_valid (const std::string& auth_header_value, const std::list<std::string>& credentials);
std::string base64_decode(const std::string& s);

std::list<std::string> split (const std::string& s, const std::string& delimiter);

};
};

#endif
