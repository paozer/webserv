#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>
#include <vector>

namespace Webserv {
namespace Utils {

std::string     join_strings_vector (const std::vector<std::string>& v, const std::string& delimiter, const std::string& endline);
int hex_to_int(const std::string &str);
int             atoi(const std::string& str);
std::string     itoa(int n);
bool            is_whitespace(char c);
void            undo_whitespace(std::string &line);
std::string     get_word(std::string &string);
void            erase_word(std::string &string);
void            erase_word(std::string &string);
char            tolower (char c);

}; // namespace Utils
}; // namespace Webserv

#endif
