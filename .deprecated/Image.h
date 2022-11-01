#pragma once
#include <cstring>
#include <iostream>
#include <vector>

#include "RHIDefinitions.h"

namespace und {
    struct Image {
        unsigned int Width{ 0 };
        unsigned int  Height{ 0 };
        unsigned int Channel{ 0 };
        unsigned char *data{ nullptr };
        size_t data_size{ 0 };
        unsigned int type;
        //unsigned int internal_format;
        //unsigned int format;
        //struct Mipmap {
        //    unsigned int  Width{ 0 };
        //    unsigned int  Height{ 0 };
        //    unsigned int Channel{ 0 };
        //    size_t offset{ 0 };
        //    size_t data_size{ 0 };
        //    Mipmap(unsigned int width, unsigned int height, size_t pitch_, size_t offset_,
        //        size_t data_size_) {
        //        Width = width;
        //        Height = height;
        //        offset = offset_;
        //        data_size = data_size_;
        //    }
        //};
        //std::vector<Mipmap> mipmaps;

        Image() = default;
        Image(const Image &rhs) = delete;  // disable copy contruct
        Image(Image &&rhs) noexcept;
        Image &operator=(const Image &rhs) = delete;  // disable copy assignment
        Image &operator=(Image &&rhs) noexcept;
        ~Image() {
            if (data) delete[] data;
        }
        EPixelFormat GetPixelFormat();
    };

    std::ostream &operator<<(std::ostream &out, const Image &image);
}  // namespace My
