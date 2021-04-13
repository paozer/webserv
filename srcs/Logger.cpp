#include "Logger.hpp"

namespace Webserv {
namespace Log {

void prepare_file()
{
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH;
    if ((fd = open("log", O_CREAT | O_RDWR | O_TRUNC, mode)) == -1){
        std::cout << "Can't create log" << std::endl;
    } 
}

std::string time_now_to_string(calendar const &now)
{
    std::string         res;

    res += "[";
    res += Utils::itoa(now.years);
    res += ".";
    if (now.days < 10)
        res += "0";
    res += Utils::itoa(now.days);
    res += ".";
    if (now.months < 10)
        res += "0";
    res += Utils::itoa(now.months);
    res += " ";
    if (now.hours < 10)
        res += "0";
    res += Utils::itoa(now.hours);
    res += "h";
    if (now.min < 10)
        res += "0";
    res += Utils::itoa(now.min);
    res += " ";
    if (now.sec < 10)
        res += "0";
    res += Utils::itoa(now.sec);
    res += "s";
    // res += Utils::itoa(now.msec);
    res += "]";
    return res;
}

std::string get_time_infos()
{
    calendar    now;

    gettimeofday(&_current_time, NULL);
    _current_time.tv_sec -= 1609459200; //Start at 1st January 2021 
    for (now.years = 2021; ; ++now.years)
    {
        if (now.years % 4){ // not a leap year
            if (_current_time.tv_sec - 31536000 > 0){
                _current_time.tv_sec -= 31536000;
            } else {
                break;
            }
        } else if (_current_time.tv_sec - 31622400 > 0) {
                _current_time.tv_sec -= 31622400;
        } else {
            break;
        }
    }
    for (now.months = 1; ; ++now.months)
    {
        if (now.months == 2) { // February
            if (_current_time.tv_sec - sec_in_a_month[now.months - 1] - (now.years % 4 ? 0 : 86400) > 0) {
                _current_time.tv_sec -= sec_in_a_month[now.months - 1] - (now.years % 4 ? 0 : 86400);
            } else {
                break;
            }
        } else if (_current_time.tv_sec - sec_in_a_month[now.months - 1] > 0) {
            _current_time.tv_sec -= sec_in_a_month[now.months - 1];
        } else {
            break;
        }
    }
    for (now.days = 1; _current_time.tv_sec - 86400 > 0; ++now.days)
        _current_time.tv_sec -= 86400;
    for (now.hours = 0; _current_time.tv_sec - 3600 > 0; ++now.hours)
        _current_time.tv_sec -= 3600;
    for (now.min = 0; _current_time.tv_sec - 60 > 0; ++now.min)
        _current_time.tv_sec -= 60;
    now.sec = _current_time.tv_sec;
    now.msec = _current_time.tv_usec / 1000;
    return (time_now_to_string(now));
}

void out(std::string const &server_name, std::string const &str)
{
    if (fd > 0){
        std::string     res(get_time_infos());
        if (server_name.size()){
            res += "[";
            res += server_name;
            res += "]";
        }
        res += " ";
        res += str;
        res += "\n";
        write(fd, res.c_str(), res.length());
    }    
}

};
};