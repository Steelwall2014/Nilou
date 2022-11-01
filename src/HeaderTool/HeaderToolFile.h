#pragma once

#include <string>
#include <vector>

namespace HeaderTool {
    class File
    {
    public:
        File() : FilePath("") { }
        File(const std::string InFileName) : FilePath(InFileName) { }
        const std::string &GetFilePath() { return FilePath; }

        template<typename Func>
        void ForEachLine(Func InFunc);

        std::vector<std::string> ReadLines();
    private:
        std::string FilePath;
    };
}
