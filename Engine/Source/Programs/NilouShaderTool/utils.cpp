#include "utils.h"

bool IsSlangModule(const std::filesystem::path& SlangFilePath)
{
    if (SlangFilePath.extension() != ".slang")
    {
        return false;
    }

    if (!std::filesystem::exists(SlangFilePath))
    {
        return false;
    }

    std::ifstream file(SlangFilePath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Remove leading whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            continue; // Skip empty lines
        }
        
        line = line.substr(start);
        
        // Skip single-line comments
        if (line.size() >= 2 && line[0] == '/' && line[1] == '/')
        {
            continue;
        }
        
        // Skip multi-line comments
        if (line.size() >= 2 && line[0] == '/' && line[1] == '*')
        {
            // Find the end of multi-line comment
            size_t endComment = line.find("*/");
            while (endComment == std::string::npos)
            {
                if (!std::getline(file, line))
                {
                    return false; // File ended but comment not closed
                }
                endComment = line.find("*/");
            }
            // Continue processing content after comment
            if (endComment + 2 < line.size())
            {
                line = line.substr(endComment + 2);
                start = line.find_first_not_of(" \t\r\n");
                if (start != std::string::npos)
                {
                    line = line.substr(start);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        
        // Check if it's a module declaration
        if (line.find("module ") == 0)
        {
            return true;
        }
        // Check if it's an implementing declaration
        else if (line.find("implementing ") == 0)
        {
            return false;
        }
        
        // If we encounter other non-empty, non-comment lines, it's not a module file
        break;
    }
    
    return false;
}