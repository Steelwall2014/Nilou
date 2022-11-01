#pragma once
#include <string>

namespace nilou {

    class FArchive
    {
    public:
        virtual FArchive &operator<<(const class FRasterizerStateInitializer &RasterizerState);
        virtual FArchive &operator>>(class FRasterizerStateInitializer &RasterizerState);

        virtual FArchive &operator<<(const class FDepthStencilStateInitializer &DepthStencilState);
        virtual FArchive &operator>>(class FDepthStencilStateInitializer &DepthStencilState);

        virtual FArchive &operator<<(const std::string &String);
        virtual FArchive &operator>>(std::string &String);
    };

}