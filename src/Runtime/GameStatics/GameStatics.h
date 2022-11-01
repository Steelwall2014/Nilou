#pragma once

#include <string>
#include <vector>

namespace GameStatics {
    
    bool StartsWith(const std::string &str, const std::string &temp);

    bool EndsWith(const std::string &str, const std::string &temp);
    
    void Trim(std::string &s);

    std::vector<std::string> Split(const std::string &s, char delim = ' ');

}