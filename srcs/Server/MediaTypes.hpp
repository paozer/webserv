#ifndef MEDIA_TYPES_HPP
#define MEDIA_TYPES_HPP

#include <map>
#include <string>

static std::map<std::string, std::string> create_media_types_map(void);
static std::map<std::string, std::string> MEDIA_TYPES = create_media_types_map();

inline std::string get_media_type (const std::string& filename)
{
    std::string extension = filename.substr(filename.rfind("."));
    std::map<std::string, std::string>::const_iterator it = MEDIA_TYPES.find(extension);
    if (it == MEDIA_TYPES.end())
        return "application/octet-stream";
    return it->second;
}

std::map<std::string, std::string> create_media_types_map (void)
{
    std::map<std::string, std::string> media_types;
    media_types[".aac"] = "audio/aac";
    media_types[".abw"] = "application/x-abiword";
    media_types[".arc"] = "application/x-freearc";
    media_types[".avi"] = "video/x-msvideo";
    media_types[".azw"] = "application/vnd.amazon.ebook";
    media_types[".bin"] = "application/octet-stream";
    media_types[".bmp"] = "image/bmp";
    media_types[".bz"] = "application/x-bzip";
    media_types[".bz2"] = "application/x-bzip2";
    media_types[".csh"] = "application/x-csh";
    media_types[".css"] = "text/css";
    media_types[".csv"] = "text/csv";
    media_types[".doc"] = "application/msword";
    media_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    media_types[".eot"] = "application/vnd.ms-fontobject";
    media_types[".epub"] = "application/epub+zip";
    media_types[".gz"] = "application/gzip";
    media_types[".gif"] = "image/gif";
    media_types[".htm"] = "text/html";
    media_types[".html"] = "text/html";
    media_types[".ico"] = "image/vnd.microsoft.icon";
    media_types[".ics"] = "text/calendar";
    media_types[".jar"] = "application/java-archive";
    media_types[".jpeg"] = "image/jpeg";
    media_types[".jpg"] = "image/jpeg";
    media_types[".js"] = "JavaScript text/javascript";
    media_types[".json"] = "application/json";
    media_types[".jsonld"] = "application/ld+json";
    media_types[".mid"] = "audio/midi audio/x-midi";
    media_types[".midi"] = "audio/midi audio/x-midi";
    media_types[".mjs"] = "text/javascript";
    media_types[".mp3"] = "audio/mpeg";
    media_types[".mpeg"] = "video/mpeg";
    media_types[".mpkg"] = "application/vnd.apple.installer+xml";
    media_types[".odp"] = "application/vnd.oasis.opendocument.presentation";
    media_types[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    media_types[".odt"] = "application/vnd.oasis.opendocument.text";
    media_types[".oga"] = "audio/ogg";
    media_types[".ogv"] = "video/ogg";
    media_types[".ogx"] = "application/ogg";
    media_types[".opus"] = "audio/opus";
    media_types[".otf"] = "font/otf";
    media_types[".png"] = "image/png";
    media_types[".pdf"] = "application/pdf";
    media_types[".php"] = "application/x-httpd-php";
    media_types[".ppt"] = "application/vnd.ms-powerpoint";
    media_types[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    media_types[".rar"] = "application/vnd.rar";
    media_types[".rtf"] = "application/rtf";
    media_types[".sh"] = "application/x-sh";
    media_types[".svg"] = "image/svg+xml";
    media_types[".swf"] = "application/x-shockwave-flash";
    media_types[".tar"] = "application/x-tar";
    media_types[".tif"] = "image/tiff";
    media_types[".tiff"] = "image/tiff";
    media_types[".ts"] = "video/mp2t";
    media_types[".ttf"] = "font/ttf";
    media_types[".txt"] = "text/plain";
    media_types[".vsd"] = "application/vnd.visio";
    media_types[".wav"] = "audio/wav";
    media_types[".weba"] = "audio/webm";
    media_types[".webm"] = "video/webm";
    media_types[".webp"] = "image/webp";
    media_types[".woff"] = "font/woff";
    media_types[".woff2"] = "font/woff2";
    media_types[".xhtml"] = "application/xhtml+xml";
    media_types[".xls"] = "application/vnd.ms-excel";
    media_types[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    media_types[".xml"] = "application/xml";
    media_types[".xul"] = "application/vnd.mozilla.xul+xml";
    media_types[".zip"] = "application/zip";
    media_types[".3gp"] = "video/3gpp";
    media_types[".3g2"] = "video/3gpp2";
    media_types[".7z"] = "application/x-7z-compressed";
    return media_types;
}

#endif
