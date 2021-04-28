# include "Configuration.hpp"

namespace Webserv {

void Configuration::parse(std::string const &file)
{
    try {
        lvector conf = Parsing::read_file(file);
        set_default(conf[0]);
        for (lvector::iterator it = ++(conf.begin()); it != conf.end(); ++it)
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
    if (print_conf)
        print_configuration();
};

Configuration::location    Configuration::load_location(std::list<std::string>::iterator &it)
{
    location        new_loc;
    std::string     key;

    new_loc._name = (*it)[(*it).length() - 1] == ' ' ? (*it).substr(0, (*it).size() - 1) : *it;
    new_loc._client_max_body_size = 0;
    new_loc._set_auto_index = false;
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
        if (key == "autoindex") {
            new_loc._autoindex = Utils::get_word(*it) == "on" ? true : false;
            new_loc._set_auto_index = true;
        } else if (key == "upload_enable") {
            new_loc._upload_enable = Utils::get_word(*it) == "on" ? true : false;
        } else if (key == "client_max_body_size") {
            new_loc._client_max_body_size = Utils::atoi(Utils::get_word(*it));
        } else if (key == "index") {
            new_loc._index = Utils::get_word(*it);
        } else if (key == "upload_path") {
            new_loc._upload_path = Utils::get_word(*it);
        } else if (key == "cgi_extension") {
            new_loc._cgi_extension = Utils::get_word(*it);
        } else if (key == "cgi_path") {
            new_loc._cgi_path.push_back(Utils::get_word(*it));
        } else if (key == "root") {
            new_loc._root = Utils::get_word(*it);
        } else if (key == "method") {
            while (!(*it).empty())
                new_loc._method.push_back(Utils::get_word(*it));
        } else if (key == "auth_basic") {
            new_loc.auth = Utils::get_word(*it);
        } else if (key == "auth_basic_user_file") {
            new_loc.auth_path = Utils::get_word(*it);
        }

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
        if (cmp == "server_name") {
            new_serv._server_name = Utils::get_word(*it);
        } else if (cmp == "error_page") {
            new_serv._error_pages.insert(std::pair<int, std::string>(Utils::atoi(Utils::get_word(*it)), Utils::get_word(*it)));
        } else if (cmp == "listen") {
            new_serv._listen = std::make_pair(Utils::atoi(Utils::get_word(*it)), Utils::get_word(*it));
            if (new_serv._listen.second == "localhost")
                new_serv._listen.second = "127.0.0.1";
        } else if (cmp == "root") {
            new_serv._root = Utils::get_word(*it);
        } else if (cmp == "auth_basic") {
            new_serv.auth = Utils::get_word(*it);
        } else if (cmp == "auth_basic_user_file") {
            new_serv.auth_path = Utils::get_word(*it);
        } else if (cmp == "location") {
            new_serv._locations.push_back(load_location(it));
        } else
            throw ConfException(*it, " Expression out of block.");
    }

    _servers.push_back(new_serv);
}

void    Configuration::complete_location(Configuration::location &loc, Configuration::server &server)
{
    if (loc._name.empty())
        loc._name = "/";
    if (loc._root.empty())
        loc._root = "";
    if (loc._index.empty())
        loc._index = "";
    if (loc.auth.empty())
        loc.auth = server.auth;
    if (loc._cgi_path.empty())
        loc._cgi_path.push_back("");
    if (loc._cgi_extension.empty())
        loc._cgi_extension = "";
    if (loc._upload_path.empty())
        loc._upload_path = "";
    if (loc._client_max_body_size == 0)
        loc._client_max_body_size = def_conf.client_max_body_size;
    if (!loc._set_auto_index)
        loc._autoindex = def_conf.auto_idx;
    if (loc.auth.empty())
        loc.auth = auth;
    if (loc.auth != "off") {
        if (loc.auth_path.empty())
            throw ConfException("Authentification", "missing credential file.");
        loc.auth_credentials = Utils::split(Files::get_file_content(loc.auth_path), "\n");
        if (loc.auth_credentials.empty())
            throw ConfException("Authentification", "bad credential file format \"<user>:<password><newline>\".");
    }
}

void    Configuration::complete_config(Configuration::server &serv)
{
    if (serv._server_name.empty())
        serv._server_name = "webserv";
    if (serv._listen.second.empty())
        serv._listen = std::pair<int, std::string>(80, "127.0.0.1");
    if (serv._root.empty())
        serv._root = "";
    if (serv.auth.empty())
        serv.auth = auth;
    if (serv._locations.empty()){
        Configuration::location default_loc;
        complete_location(default_loc, serv);
        serv._locations.push_back(default_loc);
    } else {
        for (std::vector<Configuration::location>::iterator it = serv._locations.begin(); it != serv._locations.end(); ++it)
            complete_location(*it, serv);
    }
    for (std::map<int, std::string>::iterator it = def_conf.error_pages.begin(); it != def_conf.error_pages.end(); ++it)
        if (serv._error_pages.find((*it).first) == serv._error_pages.end())
            serv._error_pages.insert(*it);
}

void    Configuration::set_default(std::list<std::string> &conf)
{
    print_conf = false;
    def_conf.auto_idx = false;
    auth = "off";
    max_connections_workers = -1;
    max_workers = -1;
    std::string tmp;
    for (std::list<std::string>::iterator it = conf.begin(); it != conf.end(); ++it)
    {
        tmp = Utils::get_word(*it);
        if (tmp == "log_enabled") {
            Log::prepare_file();
        } else if (tmp == "nb_workers") {
            max_workers = Utils::atoi(Utils::get_word(*it));
            if (max_workers < 0 || max_workers > 513)
                throw ConfException("max workers", "must be >= 0 and <= 512");
        } else if (tmp == "workers_max_connections") {
            max_connections_workers = Utils::atoi(Utils::get_word(*it));
            if (max_connections_workers < 0 || max_connections_workers > 1024)
                throw ConfException("workers's connections", "must be > 0 and <= 1024");
        } else if (tmp == "print_configuration") {
            print_conf = Utils::get_word(*it) == "on" ? true : false;
        } else if (tmp == "client_max_body_size") {
            def_conf.client_max_body_size = Utils::atoi(Utils::get_word(*it));
        } else if (tmp == "error_page") {
            def_conf.error_pages.insert(std::pair<int, std::string>(Utils::atoi(Utils::get_word(*it)), Utils::get_word(*it)));
        } else if (tmp == "autoindex") {
            def_conf.auto_idx = Utils::get_word(*it) == "on" ? true : false;
        }
    }
    if (max_workers == -1)
        max_workers =  3;
    if (max_connections_workers == -1)
        max_connections_workers = 100;
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
            std::cout << sp << "root                   : " << (*it)._root << std::endl;
            std::cout << sp << "index                  : " << (*it)._index << std::endl;
            std::cout << sp << "autoindex              : " << std::boolalpha << (*it)._autoindex << std::endl;
            std::cout << sp << "upload_enabled         : " << std::boolalpha << (*it)._upload_enable << std::endl;
            std::cout << sp << "upload_path            : " << (*it)._upload_path << std::endl;
            std::cout << sp << "client_max_body_size   : " << (*it)._client_max_body_size << std::endl;
            std::cout << sp << "cgi_extension          : " << (*it)._cgi_extension << std::endl;
            std::cout << sp << "auth_basic             : " << (*it).auth << std::endl;
            if ((*it).auth != "off")
                for (std::list<std::string>::iterator iv = (*it).auth_credentials.begin(); iv != (*it).auth_credentials.end(); ++iv) {
                    if (iv == (*it).auth_credentials.begin())
                        std::cout << sp << "auth_basic_credentials : " << *iv << std::endl;
                    else
                        std::cout << sp << "           " << *iv << std::endl;
                }
            for (std::vector<std::string>::iterator iv = (*it)._cgi_path.begin(); iv != (*it)._cgi_path.end(); ++iv)
                std::cout << sp << "cgi_path               : " << *iv << std::endl;
            std::cout << sp << "method                 :";
            for (std::vector<std::string>::iterator iv = (*it)._method.begin(); iv != (*it)._method.end(); ++iv)
                std::cout << " " << *iv;
            std::cout << std::endl;
        }
        std::cout << "----------------------------------------------------------------" << std::endl;
        std::cout << std::endl;
    }
}
};
