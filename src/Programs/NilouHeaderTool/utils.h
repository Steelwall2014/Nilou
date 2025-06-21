#pragma once
#include <string>
#include <vector>
#include <clang-c/Index.h>

std::vector<std::string> Split(const std::string &s, char delim=' ');

std::string GetCursorSpelling(CXCursor c);

std::string GetCursorKindSpelling(CXCursor c);

std::string GetCursorTypeSpelling(CXCursor c);

std::string GetTypeSpelling(CXType t);

std::string GetClangString(const CXString& str);

bool EndsWith(const std::string &str, const std::string &temp);