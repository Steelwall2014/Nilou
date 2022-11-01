#pragma once

#include <iostream>

namespace nilou {

    enum class ELogLevel
    {
        LL_Error,
        LL_Info,
        LL_Warning
    };

    #define NILOU_LOG(Level, info) \
        if constexpr (ELogLevel::LL_##Level == ELogLevel::LL_Error) \
        { \
            std::cout << "[ERROR] " << info << std::endl; \
        } \
        else if constexpr (ELogLevel::LL_##Level == ELogLevel::LL_Info) \
        { \
            std::cout << "[INFO] " << info << std::endl; \
        } \
        else if constexpr (ELogLevel::LL_##Level == ELogLevel::LL_Warning) \
        { \
            std::cout << "[WARNING] " << info << std::endl; \
        }

}