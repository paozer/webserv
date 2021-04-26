#include "Utils.hpp"

namespace Webserv {
namespace Utils {

int atoi_base(const std::string &s, const std::string& base)
{
    if (s.empty())
        return -1;
    int n = 0;
    size_t j = 0;
    std::string str = s;
    for (size_t i = 0; i < str.length(); ++i) {
        str[i] = tolower(str[i]);
        if ((j = base.find(str[i])) == std::string::npos) {
            return -1;
        } else {
            n *= base.length();
            n += j;
        }
    }
    return n;
}

int atoi(const std::string& s)
{
    size_t i = 0;
    while (i < s.length() && is_whitespace(s[i]))
        ++i;
    int sign = 1;
    if (i < s.length() && s[i] == '-') {
        sign *= -1;
        ++i;
    }
    int nb = 0;
    for (; i < s.length() && s[i] >= '0' && s[i] <= '9'; ++i)
        nb = nb * 10 + s[i] - '0';
    return nb * sign;
}

std::string itoa(int n)
{
    long int        nb = n;
    int             length = 1;
    int             sign = 1;
    std::string     res;

    if (n == 0)
        return "0";
    nb < 0 ? n = n * -1 : sign = 0;
    length = length + sign;
    while (nb /= 10) {
        res.insert(res.begin(), n % 10 + 48);
        n = n / 10;
    }
    res.insert(res.begin(), n % 10 + 48);
    if (sign != 0)
        res.insert(res.begin(), '-');
    return (res);
}

void undo_whitespace(std::string &line)
{
    for (size_t i = 0; i < line.size(); ) {
        if (i && line[i] == '#' && line[i-1] != '\n')
            line.insert(i, 1, '\n');
        else if (is_whitespace(line[i]) && ((line[i+1] && is_whitespace(line[i + 1]))
                || (i && is_whitespace(line[i - 1]))) && line[i] != '\n')
            line.erase(i, 1);
        else if (line[i] == ';')
            line.erase(i, 1);
        else
            ++i;
    }
}

std::string get_word(std::string &string)
{
    std::string res;
    size_t pos = string.find_first_of(" ");

    if (pos != std::string::npos) {
        res = string.substr(0, pos);
        erase_word(string);
    } else {
        res = string;
        string.clear();
    }
    return res;
}

std::list<std::string> split (const std::string& s, const std::string& delimiter)
{
    size_t i = 0;
    size_t j = 0;
    std::list<std::string> ret;
    while (true) {
        j = s.find(delimiter, i);
        ret.push_back(s.substr(i, j - i - 1));
        if (j == std::string::npos)
            break ;
        i = j + 1;
    }
    return ret;
}

}; // namespace Utils
}; // namespace Webserv
