#include <iostream>
#include <memory>
#include <vector>
#include <regex>
#include <sstream>

#include <glad/glad.h>

#include "Common/Log.h"
#include "Gamestatics.h"
#include "StaticMeshResources.h"
#include "Common/ContentManager.h"
#include "Material.h"
#include "RHIDefinitions.h"
#include "StaticMeshVertexBuffer.h"
#include "Texture.h"
#include "VertexFactory.h"


namespace nilou {

    bool GameStatics::StartsWith(const std::string &str, const std::string &temp)
    {
        if (str.size() < temp.size())
            return false;
        int length = std::min(str.size(), temp.size());
        for (int i = 0; i < length; i++)
        {
            if (str[i] != temp[i])
                return false;
        }
        return true;
    }

    bool GameStatics::EndsWith(const std::string &str, const std::string &temp)
    {
        if (str.size() < temp.size())
            return false;
        int length = std::min(str.size(), temp.size());
        for (int i = 0; i < length; i++)
        {
            if (str[str.size()-1 - i] != temp[temp.size()-1 - i])
                return false;
        }
        return true;
    }

    void GameStatics::Trim(std::string &s)
    {
        if( !s.empty() )
        {
            if (s[0] == ' ')
            {
                s.erase(0, s.find_first_not_of(" "));
                s.erase(s.find_last_not_of(" ") + 1);
                s.erase(0, s.find_first_not_of("\t"));
                s.erase(s.find_last_not_of("\t") + 1);
            }
            else if (s[0] == '\t')
            {
                s.erase(0, s.find_first_not_of("\t"));
                s.erase(s.find_last_not_of("\t") + 1);
                s.erase(0, s.find_first_not_of(" "));
                s.erase(s.find_last_not_of(" ") + 1);
            }
        }
    }

    static size_t find_first_not_delim(const std::string &s, char delim, size_t pos)
    {
        for (size_t i = pos; i < s.size(); i++)
            if (s[i] != delim)
                return i;
        return std::string::npos;
    }
    std::vector<std::string> GameStatics::Split(const std::string &s, char delim)
    {
        std::vector<std::string> tokens;
        size_t lastPos = find_first_not_delim(s, delim, 0);
        size_t pos = s.find(delim, lastPos);
        while (lastPos != std::string::npos)
        {
            tokens.push_back(s.substr(lastPos, pos - lastPos));
            lastPos = find_first_not_delim(s, delim, pos);
            pos = s.find(delim, lastPos);
        }
        return tokens;
    }
}
