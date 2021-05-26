#ifndef FILES_HPP
#define FILES_HPP

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>

#include <string>
#include <vector>

namespace Webserv {
namespace Files {

std::string get_file_content (const std::string& filepath);
std::string get_http_directory_listing (const std::string& path);
std::vector<std::string> get_directory_listing (const std::string& path);
int fill_with_file_content (std::string& s, const std::string& filepath);
bool create_dir(const std::string& filename);
void remove_dir(const std::string& filename);

}; // namespace Files
}; // namespace Webserv

#endif
