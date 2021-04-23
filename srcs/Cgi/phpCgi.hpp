#ifndef PHPCGI_HPP
# define PHPCGI_HPP

#include <iostream>
#include "../Http/Request.hpp"
#include "../Http/Response.hpp"
#include "../Configuration/Configuration.hpp"

#include <sys/types.h> // stat
#include <sys/stat.h>

#include <string.h> // strdup, TMP | strerror
#include <strings.h> // bzero
#include <errno.h> // errno

#include <stdio.h> // BUFSIZE

#include <map>

namespace Webserv {
class phpCgi {

	private:
		typedef Configuration::server	confServer;
		typedef Configuration::location	confLocation;
		typedef Http::Request		httpRequest;
		typedef Http::Response		httpResponse;

		typedef std::map<std::string, std::string> envMap;

	public:
		~phpCgi();
		phpCgi(	const std::string& method,
			const Configuration::server* server,
			const Configuration::location* location,
			const Http::Request& request,
			httpResponse& response,
			const std::string& filepath);

	
	private:
		phpCgi();

		std::string	getScriptName(const std::string& filepath, const confLocation* location); 
		std::string	getPathInfo(const std::string& filepath);
		std::string	getFullPath(const std::string& filepath, const confLocation* location);
		std::string	getRequestUri(const std::string& filepath, const confLocation* location);
		std::string	getQueryString(const std::string& filepath);
		int		getLenNumber(int n);
		std::string	ft_itos(const int& ipt);
		char	*assembleMetaVar(const std::string& key, const std::string& value);

		char**	_envArray;
		void setEnvArray();

		envMap	_envVar;
		void	setEnvMap(
				const std::string& method,
				const confServer* server,
				const confLocation* location,
				const httpRequest& request,
				httpResponse& response,
				const std::string& filepath);


		std::string	pullCode(std::string& value);
		bool		headersAreOut(const std::string& body);
		std::string	cutHeaders(const std::string& body, httpResponse& response);

		void		methodPost(
					httpResponse& response,
					const httpRequest& request,
					const confLocation* location,
					const std::string& filepath);



		void		methodGet(
					httpResponse& response,
					const confLocation* location,
					const std::string& filepath);

		void	pull502(httpResponse& response);

}; // class phpCgi
}; // namespace Webserv

#endif
