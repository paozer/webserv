#ifndef HTTP_CHUNKED_BODY
#define HTTP_CHUNKED_BODY

#include "Http.hpp"

#include <string>
#include <algorithm>

namespace Webserv {
namespace Http {

class ChunkedBody {
    public:
        ChunkedBody (void);
        void decode (std::string& chunk);
        inline const std::string& get_body (void) const { return body; }
        inline const std::string& get_trailer_part (void) const { return trailer_part; }
        inline const State& get_state (void) const { return state; }

    private:
        std::string body;
        std::string trailer_part;

        ssize_t curr_chunk_size;
        State state;

}; // class ChunkedBody

}; // namespace Http
}; // namespace Webserv

#endif
