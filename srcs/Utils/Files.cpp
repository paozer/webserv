#include "Files.hpp"

namespace Webserv {
namespace Utils {

int fill_with_file_content (std::string& s, const std::string& filepath)
{
    int fd;
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            char buf[stats.st_size];
            if (read(fd, &buf, stats.st_size) == stats.st_size)
                s = std::string(buf, stats.st_size);
        }
        close(fd);
        return 0;
    }
    return -1;
}

std::string get_file_content (const std::string& filepath)
{
    std::string s;
    int fd;
    if ((fd = open(filepath.c_str(), O_RDONLY)) > -1) {
        struct stat stats;
        if (fstat(fd, &stats) == 0) {
            char buf[stats.st_size];
            if (read(fd, &buf, stats.st_size) == stats.st_size)
                s = std::string(buf, stats.st_size);
        }
        close(fd);
    }
    return s;
}

std::string get_directory_listing (const std::string& path)
{
    std::string s;
    DIR * dir = opendir(path.c_str());
    for (struct dirent* dir_entry; (dir_entry = readdir(dir)) != NULL; ) {
        s += dir_entry->d_name;
        s += "\n";
    }
    closedir(dir);
    return s;
}

}; // namespace Utils
}; // namespace Files
