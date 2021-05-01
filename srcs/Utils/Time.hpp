#ifndef TIME_HPP
#define TIME_HPP

#include "Utils.hpp"
#include <sys/time.h>
#include <string>

namespace Webserv {
namespace Time {

static const int seconds_in_month[] = {
    2678400, 2419200, 2678400, 2592000, 2678400, 2592000,
    2678400, 2678400, 2592000, 2678400, 2592000, 2678400
};

static const int weekday_offset[] = { 5, 6, 0, 1, 2, 3, 4 };

std::string get_date_logger_format (void);
std::string get_http_formatted_now (void);
std::string get_http_formatted_date (time_t mtime);
std::string get_total_time(const std::string& msg, long start);
time_t get_now_time (void);
void set_tm_structure (struct tm &c, struct timeval &tv);

}; // namespace Time
}; // namespace Webserv

#endif
