#include "SerializeHelper.h"
#include "base64/base64.h"

namespace nilou {


    // static inline bool is_base64(unsigned char c) 
    // {
    //     return (isalnum(c) || (c == '+') || (c == '/'));
    // }
    // std::string SerializeHelper::Base64Encode(unsigned char const *bytes_to_encode, unsigned int in_len) 
    // {
    //     std::string Base64Header_OctetStream = "data:application/octet-stream;base64,";
    //     return Base64Header_OctetStream+base64_encode(bytes_to_encode, in_len);
    // }
    // std::string SerializeHelper::Base64Decode(std::string const &encoded_string) 
    // {
    //     std::string Base64Header_OctetStream = "data:application/octet-stream;base64,";
    //     std::string str = encoded_string.substr(Base64Header_OctetStream.size());
    //     return base64_decode(str.data(), encoded_string.size());
    // }
}