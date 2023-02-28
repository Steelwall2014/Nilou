#pragma once
#include <string>
#include <map>

#include "Platform.h"

namespace nilou {


    class FShaderCompilerEnvironment
    {
    public:
        void SetDefine(const std::string &Name)
        {
            Definitions[Name] = "";
        }

        void SetDefine(const std::string &Name, bool Value)
        {
            Definitions[Name] = Value ? "(1)" : "(0)";
        }

        void SetDefine(const std::string &Name, int32 Value)
        {
            Definitions[Name] = "("+std::to_string(Value)+")";
        }

        std::map<std::string, std::string> Definitions;
    };
}