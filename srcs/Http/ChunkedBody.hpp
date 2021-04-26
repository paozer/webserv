#ifndef HTTP_CHUNKED_BODY
#define HTTP_CHUNKED_BODY

#include "Http.hpp"

#include <string>

namespace Webserv {
namespace Http {

class ChunkedBody {
    public:
        void decode (std::string& chunk, size_t max_client_body_size);
        inline const std::string& get_body (void) const { return _body; }
        inline const std::string& get_trailer_part (void) const { return _trailer_part; }
        inline const State& get_state (void) const { return _state; }

    private:
        std::string _body;
        std::string _trailer_part;
        State _state;

}; // class ChunkedBody

}; // namespace Http
}; // namespace Webserv

#endif
