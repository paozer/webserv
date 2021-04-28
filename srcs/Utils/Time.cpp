#include "Time.hpp"

namespace Webserv {
namespace Time {

std::string get_date_header_format (void)
{
    struct tm t;
    set_tm_structure(t);
    char buf[50];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT+2", &t);
    return std::string(buf);
}

std::string get_date_logger_format (void)
{
    struct tm t;
    set_tm_structure(t);
    char buf[50];
    strftime(buf, sizeof(buf), "[%Y%d%m%H%M%S]", &t);
    return std::string(buf);
}

void set_tm_structure (struct tm &c)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec -= 1609459200; // Start at 1st January 2021
    tv.tv_sec += 7200; // Timezone adjustment

    c.tm_wday = weekday_offset[(tv.tv_sec / 86400) % 7];

    for (c.tm_year = 2021; ; ++c.tm_year) {
        if (c.tm_year % 4 && tv.tv_sec - 31536000 > 0) // not a leap year
            tv.tv_sec -= 31536000;
        else if (tv.tv_sec - 31622400 > 0)
            tv.tv_sec -= 31622400;
        else
            break;
    }
    c.tm_year -= 1900;

    for (c.tm_mon = 1; ; ++c.tm_mon) {
        // February
        if (c.tm_mon == 2 && tv.tv_sec - seconds_in_month[c.tm_mon - 1] - (c.tm_year % 4 ? 0 : 86400) > 0)
            tv.tv_sec -= seconds_in_month[c.tm_mon - 1] - (c.tm_year % 4 ? 0 : 86400);
        else if (tv.tv_sec - seconds_in_month[c.tm_mon - 1] > 0)
            tv.tv_sec -= seconds_in_month[c.tm_mon - 1];
        else
            break;
    }
    c.tm_mon -= 1;

    for (c.tm_mday = 1; tv.tv_sec > 86400; ++c.tm_mday)
        tv.tv_sec -= 86400;

    for (c.tm_hour = 0; tv.tv_sec > 3600; ++c.tm_hour)
        tv.tv_sec -= 3600;
    for (c.tm_min = 0; tv.tv_sec > 60; ++c.tm_min)
        tv.tv_sec -= 60;
    c.tm_sec = tv.tv_sec;

    c.tm_yday = 0;
    c.tm_isdst = 0;
}

};
};
