#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Configuration/Configuration.hpp"

// #include "ServerManagement.hpp"

namespace Webserv {

class CgiHandler {
	public:
		CgiHandler(const Http::Request &request, const Configuration::location *config); // sets up env according to the request
		CgiHandler(CgiHandler const &src);
		virtual ~CgiHandler(void);

		CgiHandler   	&operator=(CgiHandler const &src);
		void			executeCgi(const std::string &scriptName, Http::Response &response);	// executes cgi and returns body
	private:
		CgiHandler(void);
		void								_initEnv(const Http::Request &request, const Configuration::location &config);
		char								**_getEnvAsCstrArray() const;
		int									_getSocket(unsigned int port);
		int									_connectSocket(unsigned int port);
		std::map<std::string, std::string>	_env;
		std::string							_body;
};

};