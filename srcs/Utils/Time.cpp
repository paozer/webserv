#include "Time.hpp"

namespace Webserv {
namespace Time {

std::string get_total_time (time_t begin)
{
    int difft = difftime(get_now_time(), begin);
    int elapsed_hours = difft / 3600;
    int elapsed_minutes = (difft % 3600) / 60;
    int elapsed_seconds = difft % 60;
    std::string s;
    s += Utils::itoa(elapsed_hours) + "H ";
    s += Utils::itoa(elapsed_minutes) + "M ";
    s += Utils::itoa(elapsed_seconds) + "S ";
    return s;
}

std::string get_http_formatted_date (time_t time)
{
    char buf[50];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %T %Z", gmtime(&time));
    return std::string(buf);
}

std::string get_date_logger_format (void)
{
    time_t time = get_now_time();
    char buf[50];
    strftime(buf, sizeof(buf), "[%Y%d%m%H%M%S]", localtime(&time));
    return std::string(buf);
}

};
};
