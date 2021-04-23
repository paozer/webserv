# include "Utils.hpp"

namespace Webserv {
namespace Utils {

std::string join_strings_vector (const std::vector<std::string>& v, const std::string& delimiter, const std::string& endline)
{
    std::string str;
    for (std::vector<std::string>::const_iterator it = v.begin(); it!= v.end(); ) {
        str.append(*it);
        if (++it != v.end())
            str.append(delimiter);
        else
            str.append(endline);
    }
    return str;
}

int hex_to_int(const std::string &s)
{
    std::string str = s;
    int n = 0;
    if (str.empty())
        return -1;
    std::string base = "0123456789ABCDEF";
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] > 96 && str[i] < 123)
            str[i] -= 32;
        if (base.find(str[i]) == std::string::npos)
            return -1;
        n *= 16;
        for (size_t j = 0; j < base.length(); ++j)
            if (str[i] == base[j])
                n += j;
    }
    return n;
}

int atoi(const std::string& s)
{
    size_t i = 0;
    while (i < s.length() && ((s[i] >= 9 && s[i] <= 13) || s[i] == 32))
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

std::string         itoa(int n)
{
    long int        nb = n;
    int             lenght = 1;
    int             signe = 1;
    std::string     res;

    if (n == 0)
        return "0";
    nb < 0 ? n = n * -1 : signe = 0;
    lenght = lenght + signe;
    while (nb /= 10)
    {
        res.insert(res.begin(), n % 10 + 48);
        n = n / 10;
    }
    res.insert(res.begin(), n % 10 + 48);
    if (signe != 0)
        res.insert(res.begin(), '-');
    return (res);
}

bool    is_whitespace(char c)
{
    return ((c >= 9 && c <= 13) || c == 32);
}

void    undo_whitespace(std::string &line)
{
    for (size_t i = 0; i < line.size(); )
    {
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

void    erase_word(std::string &str)
{
    str.erase(0, str.find_first_of(" ") + 1);
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

char tolower (char c)
{
    if ('A' <= c && c <= 'Z')
        return c + 32;
    return c;
}

}; // namespace Utils
}; // namespace Webserv
