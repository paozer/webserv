#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>
#include <vector>

namespace Webserv {
namespace Utils {

std::string     join_strings_vector (const std::vector<std::string>& v, const std::string& delimiter, const std::string& endline);
int             atoi_base (const std::string& str, const std::string& base = "0123456789ABCDEF");
bool            is_whitespace(char c);
size_t          get_len(const char *s);
int		        atoi(const char *str);
std::string		itoa(int n);
void            undo_whitespace(std::string &line);

}; // namespace Utils
}; // namespace Webserv

#endif