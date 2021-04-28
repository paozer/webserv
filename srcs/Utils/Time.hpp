#ifndef TIME_HPP
#define TIME_HPP

#include "Utils.hpp"
#include <string>
#include <sys/time.h>

namespace Webserv {
namespace Time {

static const int seconds_in_month[] = {
    2678400, 2419200, 2678400, 2592000, 2678400, 2592000,
    2678400, 2678400, 2592000, 2678400, 2592000, 2678400
};

static const int weekday_offset[] = { 5, 6, 0, 1, 2, 3, 4 };

std::string get_date_header_format (void);
std::string get_date_logger_format (void);
void set_tm_structure (struct tm &t);

}; // namespace Time
}; // namespace Webserv

#endif
