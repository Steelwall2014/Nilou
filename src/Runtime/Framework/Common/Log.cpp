#include <windows.h>
#include "Log.h"

namespace nilou {

    namespace encode {
        std::string wideToMulti(std::wstring_view sourceStr, UINT pagecode)
        {
            auto newLen = WideCharToMultiByte(pagecode, 0, sourceStr.data(), sourceStr.size(), 
                                            nullptr, 0, nullptr, nullptr);

            std::string targetStr;
            targetStr.resize(newLen);
            WideCharToMultiByte(pagecode, 0, sourceStr.data(), sourceStr.size(), 
                                &targetStr[0], targetStr.size(), nullptr, nullptr);
            return targetStr;
        }

        std::string wideToUtf8(std::wstring_view sourceWStr)
        {
            return wideToMulti(sourceWStr, 65001);
        }
        std::string wideToOme(std::wstring_view sourceWStr)
        {
            return wideToMulti(sourceWStr, CP_OEMCP);
        }
    }

    std::ostream &operator<<(std::ostream &os, const wchar_t *strPt)
    {
        return os << std::wstring_view(strPt);
    }
    std::ostream &operator<<(std::ostream &os, std::wstring_view str)
    {
        return os << encode::wideToOme(str);
    }
}