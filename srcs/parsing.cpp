#include "Parsing.hpp"
#include "Utils.hpp"

namespace Webserv {

void    count_and_replace_sometimes(std::string &line)
{
    int     left = 0;
    int     right = 0;
    
    for (size_t i = 0; i < line.size(); ++i)
        if (line[i] == '{' || line[i] == '}'){
            line[i] == '{' ? ++left : ++right;
            if (line[i-1] && line[i-1] != '\n' && Utils::is_whitespace(line[i-1]))
                line.replace(i - 1, 1, "\n");
            else if (line[i+1] && line[i+1] && Utils::is_whitespace(line[i+1]))
                line.replace(i + 1, 1, "\n");
            if (left - right < 0)
                throw ParsingException(0, "Bad indentation.");
        }
    if (left != right)
        throw ParsingException(0, left > right ? "} missing." : "{ missing.");
    while (line[line.size() - 1] == '\n')
        line.erase(line.size() - 1, 1);
}

bool    check_line(std::string &line)
{
    for (int i = 0; !allowed_line_start[i].empty(); ++i)
        if (line.compare(0, allowed_line_start[i].length(), allowed_line_start[i]) == 0)
            return true;
    return false;
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
            if (!tmp.compare(allowed_methods[i]))
                find = true;
        if (!find)
            return false;
    }
    return true;
}

bool    valid_line(std::string &line)    
{
    int     space = 0;
    size_t npos = std::string::npos;

    if (line.find("method") != npos)
        return valid_methods(line);
    for (size_t i = line.size(); i > 0; --i)
        if (line[i] == ' ')
            ++space;
    if (line.find("server") != npos && line.find("server_name") == npos)
        return space ? false : true;
    if (line.find("listen") != npos || line.find("error_page") != npos)
        return  space != 2 ? false : true;
    return space > 1 ? false : true;
}

std::vector<std::list<std::string> >    parse_file(std::string &file)
{
    std::vector<std::list<std::string> >    res;
    size_t                                  pos;
    size_t                                  line = 0;
    bool                                    new_serv;

    while(!file.empty())
    {
        std::list<std::string>      serv;
        new_serv = true;
        while (!file.empty())
        {
            ++line;
            if (!check_line(file))
                throw ParsingException(line, "Unkown expression");
            if (new_serv && file.compare(0, 6, "server"))
                throw ParsingException(line, "Expect server declaration");
            if (!new_serv && !file.compare(0, 6, "server") && file.compare(0, 7, "server_"))
                break ;
            if ((pos = file.find_first_of('\n'))){
                if (file[0] != '#')
                    serv.push_back(file.substr(0, pos));
                file.erase(0, pos);
                if (file[0] == '\n')
                    file.erase(0, 1);
            } else if (file[0] != '#') {
                serv.push_back(file);
                file.clear();
            }
            if (!valid_line(serv.back()))
                throw ParsingException(line, "Bad arguments.");
            if (new_serv)
                new_serv = false;
        }
        res.push_back(serv);
    }
    return res;
}

std::vector<std::list<std::string> >    read_file(std::string file)
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
    count_and_replace_sometimes(line);
    return (parse_file(line));
}
};