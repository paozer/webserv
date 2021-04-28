#ifndef FILES_HPP
#define FILES_HPP

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <string>

namespace Webserv {
namespace Files {

std::string get_directory_listing (const std::string& path);
std::string get_file_content (const std::string& filepath);
int fill_with_file_content (std::string& s, const std::string& filepath);

}; // namespace Files
}; // namespace Webserv

#endif
