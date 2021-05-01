#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <list>
#include <stdint.h> // uint16_t

namespace Webserv {
namespace Utils {

int atoi (const std::string& str);
int atoi_base (const std::string &str, const std::string& base = "0123456789abcdef");
std::string itoa (int n);

void undo_whitespace (std::string &line);
std::string get_word (std::string &string);
std::list<std::string> split (const std::string& s, const std::string& delimiter);
inline char tolower (char c) { return c + 32 * ('A' <= c) * (c <= 'Z'); }
inline void erase_word (std::string &s) { s.erase(0, s.find_first_of(" ") + 1); }
inline bool is_whitespace (char c) { return ((c >= 9 && c <= 13) || c == 32); }

}; // namespace Utils
}; // namespace Webserv

#endif
