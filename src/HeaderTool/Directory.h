#pragma once

#include <iostream>
#include <string>
#include <filesystem>

#include "HeaderToolFile.h"
#include "GameStatics.h"

namespace HeaderTool {
    class Directory
    {
    public:
        Directory(const std::string &InDirectoryName) : DirectoryName(InDirectoryName) { }
        const std::string &GetDirectoryName() const;

        template <typename Func>
        void ForEachFile(bool bFindInChildren, Func InFunc)
        {
            if (!std::filesystem::exists(DirectoryName))
            {
                std::cout << "Directory: " + DirectoryName + " doesn't exist" << std::endl;
                return;
            }
                
            for (const std::filesystem::directory_entry & dir_entry : 
                std::filesystem::recursive_directory_iterator(DirectoryName))
            {
                if (!dir_entry.is_directory())
                {
                    std::string filepath = dir_entry.path().generic_string();
                    File file(filepath);  
                    InFunc(file);
                }
            }
        }

    private:

        std::string DirectoryName;
    };
}
