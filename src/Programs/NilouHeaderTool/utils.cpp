#include "utils.h"

using namespace std;

static size_t find_first_not_delim(const std::string &s, char delim, size_t pos)
{
    for (size_t i = pos; i < s.size(); i++)
        if (s[i] != delim)
            return i;
    return std::string::npos;
}

std::vector<std::string> Split(const std::string &s, char delim)
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

string GetCursorSpelling(CXCursor c)
{
    auto str = clang_getCursorSpelling(c);
    string s = clang_getCString(str);
    clang_disposeString(str);
    return s;
}

string GetCursorKindSpelling(CXCursor c)
{
    auto str = clang_getCursorKindSpelling(clang_getCursorKind(c));
    string s = clang_getCString(str);
    clang_disposeString(str);
    return s;
}

string GetCursorTypeSpelling(CXCursor c)
{
    auto str = clang_getTypeSpelling(clang_getCursorType(c));
    string s = clang_getCString(str);
    clang_disposeString(str);
    return s;
}

string GetTypeSpelling(CXType t)
{
    auto str = clang_getTypeSpelling(t);
    string s = clang_getCString(str);
    clang_disposeString(str);
    return s;
}

std::string GetClangString(const CXString& str) 
{
    std::string c_str = clang_getCString(str);
    clang_disposeString(str);
    return c_str;
}

bool EndsWith(const std::string &str, const std::string &temp)
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