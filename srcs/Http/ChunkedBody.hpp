#pragma once

#include "Http.hpp"

#include <string>

// *chunk == chunksize [;*chunk-ext] crlf >> chunk-data crlf
// last-chunk == 1*('0') [;*chunk-ext] crlf
// trailer-part == *(header-field) crlf
// crlf

namespace Webserv {
namespace Http {

class ChunkedBody {
    public:
        int decode (std::string chunked_body, size_t max_client_body_size = 1048576);
        void clear (void);
        const std::string& get_body (void) const { return _body; }
        const std::string& get_trailer_part (void) const { return _trailer_part; }

    private:
        std::string _body;
        std::string _trailer_part;

}; // class ChunkedBody

}; // namespace Http
}; // namespace Webserv
