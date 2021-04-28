#include "Authentication.hpp"

namespace Webserv {
namespace Http {

bool credentials_are_valid (const std::string& auth_header_value, const std::list<std::string>& credentials)
{
    size_t i = auth_header_value.find(SP);
    if (auth_header_value.compare(0, i, "Basic") != 0)
        return false;
    i = auth_header_value.find_first_not_of(SP, i); // necessary ?
    std::string decoded = base64_decode(auth_header_value.substr(i));
    return std::find(credentials.begin(), credentials.end(), decoded) != credentials.end();
}

std::string base64_decode(const std::string& encoded)
{
    std::string decoded;
    std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int num = 0;
    int count_bits = 0;

    size_t k = 0;
    for (size_t i = 0; i < encoded.length(); i += 4) {
        num = 0, count_bits = 0;
        for (size_t j = 0; j < 4; ++j) {
            if (encoded[i + j] != '=') {
                num = num << 6;
                count_bits += 6;
            }
            if ((k = charset.find(encoded[i + j])) == std::string::npos) {
                if (encoded[i + j] != '=') // malformed encoded
                    return decoded;
                num = num >> 2;
                count_bits -= 2;
            } else {
                num = num | k;
            }
        }
        while (count_bits != 0) {
            count_bits -= 8;
            decoded.push_back((num >> count_bits) & 255);
        }
    }
    return decoded;
}

};
};
