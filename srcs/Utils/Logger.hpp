#ifndef LOGGER_HPP
# define LOGGER_HPP

#include <iostream>
#include <algorithm>
#include <string>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "Utils.hpp"

namespace Webserv {
namespace Log {
static int fd = -1;

static const int sec_in_a_month[] = {
//    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 days for each months
    2678400, 2419200, 2678400, 2592000, 2678400, 2592000,
    2678400, 2678400, 2592000, 2678400, 2592000, 2678400
};

struct calendar
{
    int     years;
    int     months;
    int     days;
    int     hours;
    int     min;
    int     sec;
    int     msec;
};

static struct timeval      _current_time;

void            prepare_file();

std::string     get_time_infos();
std::string     time_now_to_string(calendar const &now);
void            out(std::string const &server_name, std::string const &str);

};
};

/*
Min			60
Hours		3600
Days		86400

Months 28	2 419 200
Months 29	2 505 600
Months 30	2 592 000
Months 31	2 678 400

Years 365	31 536 000
Years 366	31 622 400

1970 -> 2021 1st January 0h0h00
51 years (13 with 366)

51 x 31 536 000 	= 1 608 336 000
13 x 86400		=         1 123  200
			        = 1 609 459 200

Next leap year : 2024


Actual - 1 609 459 200
*/

#endif