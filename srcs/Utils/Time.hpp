#ifndef TIME_HPP
#define TIME_HPP

#include "Utils.hpp"
#include <ctime>
#include <string>

namespace Webserv {
namespace Time {

std::string get_date_logger_format (void);
std::string get_http_formatted_date (time_t mtime);
std::string get_total_time(time_t begin);
inline time_t get_now_time (void) { return time(NULL); }
inline std::string get_http_formatted_now (void) { return get_http_formatted_date(get_now_time()); }

}; // namespace Time
}; // namespace Webserv

#endif
