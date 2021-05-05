#ifndef CONTENT_NEGOTIATION_HPP
# define CONTENT_NEGOTIATION_HPP

#include "../../Utils/Files.hpp"
#include "../../Http/Request.hpp"
#include "../../Http/Response.hpp"

#include <stdio.h> // BUFSIZ
#include <errno.h>
#include <string.h> // strerror, atof
#include <map>
#include <vector>

namespace Webserv {
namespace ContentNegotiation {

typedef std::map<std::string, float> qualityMap;

typedef struct  infoFile {
    float                       quality;
    std::vector<std::string>    tags;
}               t_infoFile;

typedef std::map<std::string, t_infoFile> filesMap;

inline void insertOrAddInMap(filesMap& fileQuality, std::string path, std::string tag, float nb) {
    if (fileQuality.count(path) > 0) {
        fileQuality[path].quality += nb;
        fileQuality[path].tags.push_back(tag);
    }
    else {
        t_infoFile data;
        data.quality = nb;
        data.tags.push_back(tag);
        fileQuality.insert(std::pair<std::string, t_infoFile>(path, data));
    }
}

std::string pullBody(const std::string& path);
std::string getTagValue(const std::string& body, const std::string& search);
void        checkingInFile(std::string path, const std::string &balise, const qualityMap& refQuality, qualityMap& fileQuality);
std::string selectFile(const Http::Request& request, Http::Response& response, const std::string& filepath);
qualityMap& parseContentHeader(qualityMap& contentQuality, std::string header);

struct  errorFile: public std::exception {};

}; // ContentNegotiation
}; // namespace Webserv

#endif
