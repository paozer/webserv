#include "Files.hpp"

namespace Webserv {
namespace Files {

int fill_with_file_content (std::string& s, const std::string& filepath)
{
    int fd;
    struct stat stats;
    if (stat(filepath.c_str(), &stats) == 0 &&
            (fd = open(filepath.c_str(), O_RDONLY)) != -1) {
        char buf[stats.st_size];
        int ret = read(fd, &buf, stats.st_size);
        close(fd);
        if (ret == stats.st_size) {
            s = std::string(buf, stats.st_size);
            return 0;
        }
    }
    return -1;
}

std::string get_file_content (const std::string& filepath)
{
    std::string s;
    fill_with_file_content(s, filepath);
    return s;
}

std::vector<std::string> get_directory_listing (const std::string& path)
{
    std::vector<std::string> dl;
    DIR * dir = opendir(path.c_str());
    if (dir != NULL) {
        for (struct dirent* dir_entry; (dir_entry = readdir(dir)) != NULL; )
            dl.push_back(dir_entry->d_name);
        closedir(dir);
    }
    return dl;
}

std::string get_http_directory_listing (const std::string& path)
{
    std::string s = "<html>\n<body>\n<p>\n";
    std::vector<std::string> dl = get_directory_listing(path);
    for (std::vector<std::string>::const_iterator it = dl.begin(); it != dl.end(); ++it)
        s += *it + "<br>\n";
    s += "</p>\n</body>\n</html>";
    return s;
}

bool create_dir(const std::string& filename)
{
    if (!(mkdir(filename.c_str(), 0777)))
        return true;
    return errno == EEXIST;
}

void remove_dir(const std::string& filename)
{
    std::vector<std::string> file = get_directory_listing(filename);
    if (!file.empty())
        for (std::vector<std::string>::iterator it = file.begin(); it != file.end(); ++it)
            unlink((filename + "/" + *it).c_str());
    rmdir(filename.c_str());
}

}; // namespace Files
}; // namespace Webserv
