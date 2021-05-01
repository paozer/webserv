#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Configuration/Configuration.hpp"

namespace Webserv {

class CgiHandler {
    public:
        CgiHandler(const Http::Request &request, const Configuration::location *config); // sets up env according to the request
        void            executeCgi(const std::string &scriptName, Http::Response &response);    // executes cgi and returns body

    private:
        void                                _initEnv(const Http::Request &request, const Configuration::location &config);
        char                                **_getEnvAsCstrArray() const;
        int                                 _getSocket(unsigned int port);
        int                                 _connectSocket(unsigned int port);
        std::map<std::string, std::string>  _env;
        std::string                         _body;
};

};
