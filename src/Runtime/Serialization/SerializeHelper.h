#pragma once
#include <string>
#include <json/json.hpp>
#include <magic_enum.hpp>

#include "Archive.h"

namespace nilou {

    class SerializeHelper
    {
    public:
        static std::string Base64Encode(unsigned char const *bytes_to_encode, unsigned int in_len);
        static std::string Base64Decode(std::string const &encoded_string);
        static bool CheckIsType(nlohmann::json &json, const std::string &Name)
        {
            return json.contains("ClassName") && json["ClassName"] == Name;
        }
    };

    template<typename T>
    class TStaticSerializer
    {
    public:
        static void Serialize(const T &Object, nlohmann::json &json, FArchiveBuffers &Buffers) 
        { 
        }
        static void Deserialize(T &Object, nlohmann::json &json, void* Buffer) 
        { 
        }
    };

}