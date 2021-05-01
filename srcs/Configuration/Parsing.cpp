#include "Parsing.hpp"

namespace Webserv {
namespace Parsing {

void    check_blocks(std::string const &line)
{
    int left = 0;
    int right = 0;

    for (size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '{' || line[i] == '}')
            line[i] == '{' ? ++left : ++right;
        if (left - right < 0)
                throw ParsingException(0, "Bad indentation.");
    }
    if (left != right)
        throw ParsingException(0, left > right ? "'}' missing." : "'{' missing.");
}

bool    compare_first_word(std::string const &base, std::string const &to_compare)
{
   if (base.length() > to_compare.length())
        return false;
    if (to_compare.compare(0, base.length(), base))
        return false;
    if (to_compare.length() > base.length() && !(Utils::is_whitespace(to_compare[base.length()])))
        return false;
    return true;
}

int     word_before_block(std::string const &str)
{
    int     word = 0;
    for (int i = 0; str[i]; ++i)
    {
        if (str[i] == '{')
            return word;
        if (str[i] == ' ' || str[i] == '\n')
            ++word;
    }
    return -1;
}

bool    check_line(std::string const &line)
{
    if (line[0] == '#')
        return true;
    for (int i = 0; !allowed_line_start[i].empty(); ++i)
        if (compare_first_word(allowed_line_start[i], line))
            return true;
    return false;
}

int     count_block(std::string const &file, bool &new_serv)
{
    int     count = 0;

    for (int i = 0; file[i] && file[i] != '\n'; ++i) {
        if (file[i] == '{') {
            if (new_serv){
                new_serv = false;
            } else {
                ++count;
            }
        } else if (file[i] == '}') {
            --count;
        }
    }
    return count;
}

void    adjust_block(std::list<std::string> &serv)
{
    for (std::list<std::string>::iterator it = serv.begin(); it != serv.end(); ++it)
    {
        for (int idx = 0; (*it)[idx]; ++idx){
            if (idx == 0)
                continue ;
            if ((*it)[idx] == '{' || (*it)[idx] == '}'){
                serv.insert(++it, (*--it).substr(idx, std::string::npos));
                (*it).erase(Utils::is_whitespace((*it)[idx]) ? idx - 1 : idx);
                break ;
            }
        }
    }
}

bool    valid_methods(std::string line)
{
    size_t          pos;
    std::string     tmp;
    bool            find;

    line.erase(0, 7);
    if (line.empty())
        return false;
    while (!line.empty())
    {
        find = false;
        pos = line.find_first_of(' ');
        if (pos == std::string::npos){
            tmp = line;
            line.clear();
        } else {
            tmp = line.substr(0, pos);
            line.erase(0, pos + 1);
        }
        for (int i = 0; !allowed_methods[i].empty(); ++i)
            if (!tmp.compare(allowed_methods[i])) {
                if (tmp == "CONNECT" || tmp == "TRACE")
                    throw ConfException(line, tmp + " is not allowed on this server.");
            find = true;
            }
        if (!find)
            return false;
    }
    return true;
}

bool    check_auth(std::string &line)
{
    int i = line.find_first_of(' ') + 1;
    int j = line.length() - 1;
    if (line[i++] != '\"' || line[j] != '\"')
        return false;
    for (; i < j; ++i)
        if (!(line[i] > 31) * (line[i] < 123))
            return false;
    return true;
}

bool    valid_line(std::string &line)
{
    int     space = 0;

    if (compare_first_word("method", line))
        return valid_methods(line);
    for (size_t i = line.size(); i > 0; --i)
        if (line[i] == ' ' && line[i+1] != '{' && line[i+1] != '}')
            ++space;
    if (compare_first_word("server", line))
        return !space;
    if (compare_first_word("listen", line) || compare_first_word("error_page", line))
        return  space == 2 ;
    if (line.length() == 1 && (line == "}" || line == "{"))
        return !space;
    if (compare_first_word("auth_basic", line))
        return check_auth(line);
    return space == 1;
}

void        push_a_line(std::list<std::string> &serv, std::string &line, int nb_line)
{
    size_t      pos;

    if (!check_line(line))
        throw ParsingException(nb_line, "Unkown expression");
    if ((pos = line.find_first_of('\n'))){
        if (line[0] != '#')
            serv.push_back(line.substr(0, pos));
            line.erase(0, pos);
        if (line[0] == '\n')
            line.erase(0, 1);
    } else if (line[0] != '#') {
        serv.push_back(line);
        line.clear();
    }
    if (!valid_line(serv.back()))
        throw ParsingException(nb_line, "Bad arguments.");
}

bool    parse_line_out_of_blocks(std::list<std::string> &conf, std::string &line, int nb_line)
{
    for (int i = 0; !allowed_line_out[i].empty(); ++i)
        if (compare_first_word(allowed_line_out[i], line)) {
            push_a_line(conf, line, nb_line);
            return true;
        }
    return false;
}

std::vector<std::list<std::string> >    parse_file(std::string &file, int fd)
{
    std::vector<std::list<std::string> >    res;
    size_t                                  line = 1;
    bool                                    new_serv;

    std::list<std::string> out_block;
    res.push_back(out_block);
    while (!file.empty())
    {
        if (file[0] == '#') {
            file.erase(0, file.find_first_of('\n') + 1);
            ++line;
        } else if (compare_first_word("server", file)) {
            if (word_before_block(file) != 1)
                throw ParsingException(line, "Bad Server Declaration");
            std::list<std::string>      serv;
            new_serv = true;
            for (int bc = 1; bc; ++line)
            {
                bc += count_block(file, new_serv);
                push_a_line(serv, file, line);
            }
            adjust_block(serv);
            res.push_back(serv);
        } else if (parse_line_out_of_blocks(res[0], file, line)) {
            ++line;
        } else {
            throw ParsingException(line, "Unkown expression out of block");
        }
    }
    close(fd);
    return res;
}

std::vector<std::list<std::string> >    read_file(std::string const &file)
{
    int                         fd;
    int                         pos;
    char                        buffer[BUFFER_SIZE + 1];
    std::string                 line;

    if (file.find(".conf") == std::string::npos)
        throw ParsingException(0, "Need a .conf file.");
    if ((fd = open(file.c_str(), O_RDONLY)) == -1)
        throw ParsingException(0, "Bad config file name.");
    while ((pos = read(fd, buffer, BUFFER_SIZE)) > 0)
    {
        buffer[pos] = 0;
        line += buffer;
    }
    Utils::undo_whitespace(line);
    check_blocks(line);
    return (parse_file(line, fd));
}

}; //namespace Parsing
}; //namespace Webserv
