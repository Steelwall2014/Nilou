#include "HeaderToolFile.h"
#include <fstream>
#include <ios>
#include <string>
#include <vector>

namespace HeaderTool {
    template<typename Func>
    void File::ForEachLine(Func InFunc)
    {
        std::ifstream stream(FilePath);

        char str_line[260];
        stream.getline(str_line, 260);
        InFunc(std::string(str_line));
        while (!stream.eof())
        {
            stream.getline(str_line, 260);
            InFunc(std::string(str_line));
        }
    }

    std::vector<std::string> File::ReadLines()
    {
        std::vector<std::string> res;
        std::ifstream stream(FilePath);

        char str_line[260];
        stream.getline(str_line, 260);
        res.push_back(str_line);
        while (!stream.eof())
        {
            stream.getline(str_line, 260);
            res.push_back(str_line);
        }
        return res;
    }
}