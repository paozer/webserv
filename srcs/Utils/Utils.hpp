#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <list>
#include <stdint.h> // uint16_t

namespace Webserv {
namespace Utils {

int atoi_base(const std::string &str, const std::string& base = "0123456789abcdef");
int atoi(const std::string& str);
std::string itoa(int n);

void undo_whitespace(std::string &line);
inline char tolower (char c) { return c + 32 * ('A' <= c) * (c <= 'Z'); }
inline void erase_word(std::string &s) { s.erase(0, s.find_first_of(" ") + 1); }
inline bool is_whitespace(char c) { return ((c >= 9 && c <= 13) || c == 32); }
std::string get_word(std::string &string);
std::list<std::string> split (const std::string& s, const std::string& delimiter);

inline uint16_t m_htons(uint16_t v) { return (v >> 8) | (v << 8); }
inline std::string inet_ntoa(unsigned int s_addr) {
    unsigned char bytes[4];
    bytes[0] = s_addr & 0xFF;
    bytes[1] = (s_addr >> 8) & 0xFF;
    bytes[2] = (s_addr >> 16) & 0xFF;
    bytes[3] = (s_addr >> 24) & 0xFF;
    return std::string(itoa(bytes[0]) + "." + itoa(bytes[1]) + "." + itoa(bytes[2]) + "." + itoa(bytes[3]));
}

}; // namespace Utils
}; // namespace Webserv

#endif
