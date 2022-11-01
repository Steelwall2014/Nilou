#include <glad/glad.h>
#include "Image.h"

using namespace std;

namespace und {
    Image::Image(Image &&rhs) noexcept {
        Width = rhs.Width;
        Height = rhs.Height;
        data = rhs.data;
        //bitcount = rhs.bitcount;
        //pitch = rhs.pitch;
        data_size = rhs.data_size;
        //compressed = rhs.compressed;
        //is_float = rhs.is_float;
        //compress_format = rhs.compress_format;
        //mipmaps = std::move(rhs.mipmaps);
        rhs.Width = 0;
        rhs.Height = 0;
        rhs.data = nullptr;
        //rhs.bitcount = 0;
        //rhs.pitch = 0;
        rhs.data_size = 0;
        //rhs.compressed = false;
        //rhs.is_float = false;
        //rhs.compress_format = 0;
    }

    Image &Image::operator=(Image &&rhs) noexcept {
        if (this != &rhs) {
            Width = rhs.Width;
            Height = rhs.Height;
            data = rhs.data;
            //bitcount = rhs.bitcount;
            //pitch = rhs.pitch;
            data_size = rhs.data_size;
            //compressed = rhs.compressed;
            //is_float = rhs.is_float;
            //compress_format = rhs.compress_format;
            //mipmaps = std::move(rhs.mipmaps);
            rhs.Width = 0;
            rhs.Height = 0;
            rhs.data = nullptr;
            //rhs.bitcount = 0;
            //rhs.pitch = 0;
            rhs.data_size = 0;
            //rhs.compressed = false;
            //rhs.is_float = false;
            //rhs.compress_format = 0;
        }
        return *this;
    }

    EPixelFormat Image::GetPixelFormat()
    {
        EPixelFormat format = EPixelFormat::PF_R8G8B8A8;

        // if (type == GL_BYTE)
        // {
        //     if (this->Channel == 4)
        //         format = GL_RGBA8I;
        //     else if (this->Channel == 3)
        //         format = GL_RGB8I;
        //     else
        //         throw("not implemented");
        // }
        if (type == GL_UNSIGNED_BYTE)
        {
            if (this->Channel == 4)
                format = EPixelFormat::PF_R8G8B8A8;
            else if (this->Channel == 3)
                format = EPixelFormat::PF_R8G8B8;
            else
                throw("not implemented");
        }
        // else if (type == GL_SHORT)
        // {
        //     if (this->Channel == 4)
        //         format = PF_R16;
        //     else if (this->Channel == 3)
        //         format = GL_RGB16I;
        //     else if (this->Channel == 2)
        //         format = GL_RG16I;
        //     else if (this->Channel == 1)
        //         format = GL_R16I;
        // }
        // else if (type == GL_UNSIGNED_SHORT)
        // {
        //     if (this->Channel == 4)
        //         format = GL_RGBA16UI;
        //     else if (this->Channel == 3)
        //         format = GL_RGB16UI;
        //     else if (this->Channel == 2)
        //         format = GL_RG16UI;
        //     else if (this->Channel == 1)
        //         format = GL_R16UI;
        // }
        // else if (type == GL_INT)
        // {
        //     if (this->Channel == 4)
        //         format = GL_RGBA32I;
        //     else if (this->Channel == 3)
        //         format = GL_RGB32I;
        //     else
        //         throw("not implemented");
        // }
        // else if (type == GL_UNSIGNED_INT)
        // {
        //     if (this->Channel == 4)
        //         format = GL_RGBA32UI;
        //     else if (this->Channel == 3)
        //         format = GL_RGB32UI;
        //     else
        //         throw("not implemented");
        // }
        else if (type == GL_FLOAT)
        {
            if (this->Channel == 4)
                format = EPixelFormat::PF_R32G32B32A32F;
            // else if (this->Channel == 3)
            //     format = GL_RGB32F;
            else if (this->Channel == 2)
                format = EPixelFormat::PF_R32G32F;
            else if (this->Channel == 1)
                format = EPixelFormat::PF_R32F;
            else
                throw("not implemented");
        }
        else if (type == GL_HALF_FLOAT)
        {
            if (this->Channel == 4)
                format = EPixelFormat::PF_R16G16B16A16F;
            // else if (this->Channel == 3)
            //     format = GL_RGB16F;
            else if (this->Channel == 2)
                format = EPixelFormat::PF_R16G16F;
            else if (this->Channel == 1)
                format = EPixelFormat::PF_R16F;
            else
                throw("not implemented");
        }
        else
            throw("not implemented");
        return format;
    }

    ostream &operator<<(ostream &out, const Image &image) {
        out << "Image" << endl;
        out << "-----" << endl;
        out << "Width: " << image.Width << endl;
        out << "Height: " << image.Height << endl;
        //out << "Bit Count: " << image.bitcount << endl;
        //out << "Pitch: " << image.pitch << endl;
        out << "Data Size: " << image.data_size << endl;

#if DUMP_DETAILS
        int byte_count = image.bitcount >> 3;

        for (uint32_t i = 0; i < image.Height; i++) {
            for (uint32_t j = 0; j < image.Width; j++) {
                for (auto k = 0; k < byte_count; k++) {
                    printf("%x ",
                        reinterpret_cast<uint8_t *>(
                            image.data)[image.pitch * i + j * byte_count + k]);
                }
                cout << "\t";
            }
            cout << endl;
        }
#endif

        return out;
    }
}
