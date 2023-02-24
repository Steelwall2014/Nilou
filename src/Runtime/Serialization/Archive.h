#pragma once
#include <string>

#include <json/json.hpp>

namespace nilou {

    class FArchive
    {
    public:
        nlohmann::json json;
    };

}