#include "ContentNegotiation.hpp"

namespace Webserv {
namespace ContentNegotiation {

std::string pullBody(const std::string& path) {
    int fd = 0;
    std::string body;
    body.reserve(BUFSIZ);

    if ((fd = open(path.c_str(), O_RDONLY)) < 0) {
        Log::out(path, strerror(errno));
        throw ContentNegotiation::errorFile();
    }

    char buf[BUFSIZ];
    for (int ret = 1; ret > 0; body.append(buf, ret))
        if ((ret = read(fd, buf, BUFSIZ)) < 0)
            throw ContentNegotiation::errorFile();

    close(fd);
    return (body);
}

std::string getTagValue(const std::string& body, const std::string& search) {
    std::size_t begin = body.find(search);
    if (begin == std::string::npos)
        return ("");
    else {
        begin += search.length();
        return (body.substr(begin, body.find("\"", begin) - begin));
    }
}

void        checkingInFile(std::string path, const std::string &tag, const qualityMap& refQuality, filesMap& fileQuality) {
    if (tag == "") {
        if (refQuality.count("*") > 0)
            insertOrAddInMap(fileQuality, path, tag, refQuality.find("*")->second);
        else
            insertOrAddInMap(fileQuality, path, tag, 0);
        return;
    }

    for (qualityMap::const_iterator it = refQuality.begin(); it != refQuality.end(); ++it) {
        if (it->first.find(tag) != std::string::npos) {
            insertOrAddInMap(fileQuality, path, tag, it->second);
            return;
        }
    }

    if (refQuality.count("*") > 0)
        insertOrAddInMap(fileQuality, path, tag, refQuality.find("*")->second);
    else
        insertOrAddInMap(fileQuality, path, tag, 0);
} // checkingFile

std::string selectFile(const Http::Request& request, Http::Response& response, const std::string& filepath) {
    std::string ret = filepath;

    bool    languageOn = request.has_header("accept-language");;
    bool    charsetOn = request.has_header("accept-charset");

    std::string name = ret.substr(ret.rfind("/") + 1);
    std::list<std::string> files = Files::get_directory_listing(ret.substr(0, ret.rfind("/")));

    for (std::list<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
        if (it->find(name) == std::string::npos) {
            std::list<std::string>::iterator tmp = it;
            --tmp;
            files.erase(it);
            it = tmp;
        }
    }

    if (files.size() <= 1)
        return (filepath);

    qualityMap langQuality;
    qualityMap charsetQuality;
    if (languageOn)
        langQuality = parseContentHeader(langQuality, request.get_header_values("accept-language"));
    if (charsetOn)
        charsetQuality = parseContentHeader(charsetQuality, request.get_header_values("accept-charset"));

    filesMap fileQuality;
    std::string body;
    std::string tag;
    std::string path;
    for (std::list<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
        path = ret.substr(0, ret.rfind("/") + 1);
        path += *it;
        try {
            body = Files::get_file_content(path);
            if (languageOn) {
                tag = getTagValue(body, "<html lang=\"");
                checkingInFile(path, tag, langQuality, fileQuality);
            }
            if (charsetOn) {
                tag = getTagValue(body, "<meta charset=\"");
                checkingInFile(path, tag, charsetQuality, fileQuality);
            }
        }
        catch (ContentNegotiation::errorFile) {
            Log::out(path, "File couldn't be opened");
            continue;
        }
    }

    // Select file with higher quality.
    filesMap::iterator returnFile = fileQuality.begin();
    for (filesMap::iterator itMap = fileQuality.begin(); itMap != fileQuality.end(); ++itMap) {
        if (returnFile->second.quality < itMap->second.quality)
            returnFile = itMap;
    }

    int itVector = 0;
    if (languageOn && returnFile->second.tags[itVector].length() != 0)
        response.append_header("Content-language", returnFile->second.tags[itVector++]);
    if (charsetOn && returnFile->second.tags[itVector].length() != 0)
        response.append_header("Content-charset", returnFile->second.tags[itVector]);
    return (returnFile->first);
} // selectFile

float   ft_atof(const std::string& input) {
    if (input.find(".") == std::string::npos || input.substr(0, input.find(".")) == "1")
        return (1);

    std::string str_fract = input.substr(input.find(".") + 1, input.find_first_not_of("0123456789"));
    float fract = Utils::atoi(str_fract);
    int pow = 10;
    for (std::size_t i = 0; i < str_fract.size(); ++i)
        fract /= pow;
    return (fract);
}

qualityMap& parseContentHeader(qualityMap& contentQuality, std::string header) {
    std::string key;
    float       value = 0;
    std::size_t start;
    std::size_t end;
    while (!header.empty()) {
        key = header.substr(0, header.find(";"));
        start = header.find("q=");
        end = header.find(",", start);
        if (end != std::string::npos) {
            value = ft_atof(header.substr(start + 2, end - start).c_str());
            header = header.substr(end + 1);
        }
        else {
            value = ft_atof(header.substr(start + 2, header.length() - start).c_str());
            header.clear();
        }
        contentQuality.insert(std::pair<std::string, float>(key, value));
    }
    return (contentQuality);
} // parseContentHeader

}; // namespace ContentNegotiation
}; // namespace Webserv
