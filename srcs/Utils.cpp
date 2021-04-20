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

int atoi_base (const std::string& str, const std::string& base)
{
    if (str.empty())
        return -1;
    int n = 0;
    size_t i = 0;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if ((i = base.find(*it)) != std::string::npos) {
            n *= 10;
            n += i;
        } else {
            return -1;
        }
    }
    return n;
}

bool    is_whitespace(char c)
{
    if ((c >= 9 && c <= 13) || c == 32)
        return true;
    return false;
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

int		atoi(const char *str)
{
	int		i;
	int		nb;
	int		sign;

	sign = 1;
	i = 0;
	nb = 0;
	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
		i++;
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			sign *= -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
		nb = nb * 10 + (int)str[i++] - '0';
	return (nb * sign);
}

std::string			itoa(int n)
{
	long int		nb = n;
	int	        	lenght = 1;
	int     		signe = 1;
	std::string     res;

    if (n == 0)
        return "0";
    nb < 0 ? n = n * -1 : signe = 0;
	lenght = lenght + signe;
	while (nb /= 10)
	{
		res.insert(res.begin(),  n % 10 + 48);
		n = n / 10;
	}
    res.insert(res.begin(),  n % 10 + 48);
	if (signe != 0)
		res.insert(res.begin(), '-');
	return (res);
}

void    erase_word(std::string &string)
{
    size_t  pos = string.find_first_of(" ");
    string.erase(0, pos + 1);
}

std::string get_word(std::string &string)
{
    std::string res;
    size_t npos = std::string::npos;
    size_t pos = string.find_first_of(" ");

    if (pos != npos){
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
