#ifndef FILES_HPP
#define FILES_HPP

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <string>
#include <list>

namespace Webserv {
namespace Files {

std::string get_file_content (const std::string& filepath);
std::string get_http_directory_listing (const std::string& path);
std::list<std::string> get_directory_listing (const std::string& path);
int fill_with_file_content (std::string& s, const std::string& filepath);

}; // namespace Files
}; // namespace Webserv

#endif
