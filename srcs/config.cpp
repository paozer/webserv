# include "Config.hpp"

namespace Webserv {

Configuration::Configuration() {};

Configuration::Configuration(std::string const &file)
{
    try {
        lvector conf = Parsing::read_file(file);
        for (lvector::iterator it = conf.begin(); it != conf.end(); ++it)
           load_config(*it);
        for (std::vector<Configuration::server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
            complete_config(*it);
    } catch (Webserv::Parsing::ParsingException const &e){
        std::cout << e.what() << std::endl;
        exit(0);
    } catch (Webserv::ConfException const &e){
        std::cout << e.what() << std::endl;
        exit(0);
    }
};

Configuration::Configuration(const Configuration &other)
{
    *this = other;
}

Configuration& Configuration::operator=(const Configuration &other)
{
    if (this != &other)
        _servers = other._servers;
    return *this;
}

Configuration::location    Configuration::load_location(std::list<std::string>::iterator &it)
{
    location        new_loc;
    std::string     key;

    new_loc._name = *it;
    new_loc._client_max_body_size = 0;
    new_loc._autoindex = false;
    new_loc._upload_enable = false;
    ++it;
    if ((*it).compare("{"))
            throw ConfException("Location " + new_loc._name, " Bad indentation.");
    ++it;
    while ((*it).compare("}"))
    {
        int count = 0;
        for (int i = 0; !allowed_location[i].empty(); ++i)
            if ((*it).compare(0, allowed_location[i].length(), allowed_location[i]) == 0)
                ++count;
        if (!count)
            throw ConfException("At: " + Utils::get_word(*it) + " " + Utils::get_word(*it), "\tUnkown expression.");
        key = Utils::get_word(*it);
        if (key == "autoindex")
            new_loc._autoindex = Utils::get_word(*it) == "on" ? true : false;
        else if (key == "upload_enable")
            new_loc._upload_enable = Utils::get_word(*it) == "on" ? true : false;
        else if (key == "client_max_body_size")
            new_loc._client_max_body_size = Utils::atoi(Utils::get_word(*it).c_str());
        else if (key == "index")
            new_loc._index = Utils::get_word(*it);
        else if (key == "upload_path")
            new_loc._upload_path = Utils::get_word(*it);
        else if (key == "cgi_extension")
            new_loc._cgi_extension = Utils::get_word(*it);
        else if (key == "cgi_path")
            new_loc._cgi_path.push_back(Utils::get_word(*it));
        else if (key == "method")
            while (!(*it).empty())
                new_loc._method.push_back(Utils::get_word(*it));
        ++it;
    }
    return new_loc;
}

void    Configuration::load_config(std::list<std::string> &conf)
{
    server          new_serv;
    std::string     cmp;

    new_serv._id = _servers.size(); 
    for (std::list<std::string>::iterator it = ++conf.begin(); it != conf.end(); ++it)
    {
        if (!(*it).compare("{") || !(*it).compare("}"))
            continue;
        cmp = Utils::get_word(*it);
        if (cmp == "server_name")
            new_serv._server_name = Utils::get_word(*it);
        else if (cmp == "error_page")
            new_serv._error_pages.insert(std::pair<int, std::string>(Utils::atoi(Utils::get_word(*it).c_str()), Utils::get_word(*it)));
        else if (cmp == "listen")
            new_serv._listen = std::pair<int, std::string>(Utils::atoi(Utils::get_word(*it).c_str()), Utils::get_word(*it));
        else if (cmp == "root")
            new_serv._root = Utils::get_word(*it);
        else if (cmp == "location")
            new_serv._locations.push_back(load_location(it));
        else
            throw ConfException(*it, " Expression out of block.");
    }
    _servers.push_back(new_serv);
}

void    Configuration::complete_location(Configuration::location &loc)
{
    if (loc._name.empty())
        loc._name = "/";
    if (loc._root.empty())
        loc._root = "";
    if (loc._index.empty())
        loc._index = "";
    if (loc._cgi_path.empty())
        loc._cgi_path.push_back("");
    if (loc._cgi_extension.empty())
        loc._cgi_extension = "";
    if (loc._upload_path.empty())
        loc._upload_path = "";
    if (loc._client_max_body_size == 0)
        loc._client_max_body_size = 100000;
}

void    Configuration::complete_config(Configuration::server &serv)
{
    if (serv._server_name.empty())
        serv._server_name = "webserv";
    if (serv._listen.second.empty())
        serv._listen = std::pair<int, std::string>(80, "127.0.0.1");
    if (serv._root.empty())
        serv._root = "";
    if (serv._locations.empty()){
        Configuration::location default_loc;
        complete_location(default_loc);
        serv._locations.push_back(default_loc);
    } else {
        for (std::vector<Configuration::location>::iterator it = serv._locations.begin(); it != serv._locations.end(); ++it)
            complete_location(*it);
    }
}

/***********************************************************************
**                             GETTERS                                **
***********************************************************************/

Configuration::server*     Configuration::get_server(int id)
{
    for (std::vector<struct server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
        if ((*it)._id == id)
            return &(*it);
    return NULL;
}

Configuration::location*   Configuration::get_locations(Configuration::server &serv, std::string &name)
{
    for (std::vector<Configuration::location>::iterator it = serv._locations.begin(); it != serv._locations.end(); ++it)
        if ((*it)._name == name)
            return &(*it);
    return NULL;
}

void                    Configuration::print_configuration()
{
    std::cout << "\n                 --> Number of Servers : " <<  get_nb_server()  << " <--\n" << std::endl;
    for (std::vector<Configuration::server>::iterator actual = _servers.begin(); actual != _servers.end(); ++actual)
    {
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "                       ~ Config Server ~                        " << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "Id        : " << (*actual)._id << std::endl;
        std::cout << "Serv Name : " << (*actual)._server_name << std::endl;
        std::cout << "Root      : " << (*actual)._root << std::endl;
        std::cout << "Listen on : " << (*actual)._listen.first << " " << (*actual)._listen.second << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "Err Page:  ";
        for (std::map<int, std::string>::iterator it = (*actual)._error_pages.begin(); it != (*actual)._error_pages.end(); ++it){
            if (it == (*actual)._error_pages.begin())
                std::cout << (*it).first << " " << (*it).second << std::endl;
            else
                std::cout << "           " <<  (*it).first << " " << (*it).second << std::endl;
        }
        if ((*actual)._error_pages.empty())
            std::cout << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << "Location:  ";
        for (std::vector<Configuration::location>::iterator it = (*actual)._locations.begin(); it != (*actual)._locations.end(); ++it){
            std::string sp = "                 ";
            if (it != (*actual)._locations.begin())
                std::cout << "           ";
            std::cout << (*it)._name << std::endl;
            std::cout << sp << "root                  : " << (*it)._root << std::endl;
            std::cout << sp << "index                 : " << (*it)._index << std::endl;
            std::cout << sp << "autonindex            : " << std::boolalpha << (*it)._autoindex << std::endl;
            std::cout << sp << "upload_enable         : " << std::boolalpha << (*it)._upload_enable << std::endl;
            std::cout << sp << "upload_path           : " << (*it)._upload_path << std::endl;
            std::cout << sp << "client_max_body_size  : " << (*it)._client_max_body_size << std::endl;
            std::cout << sp << "cgi_extension         : " << (*it)._cgi_extension << std::endl;
            for (std::vector<std::string>::iterator iv = (*it)._cgi_path.begin(); iv != (*it)._cgi_path.end(); ++iv)
                std::cout << sp << "cgi_path              : " << *iv << std::endl;
            std::cout << sp << "method                :";
            for (std::vector<std::string>::iterator iv = (*it)._method.begin(); iv != (*it)._method.end(); ++iv)
                std::cout << " " << *iv;
            std::cout << std::endl;
        }
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << std::endl;
    }
}
}; 
