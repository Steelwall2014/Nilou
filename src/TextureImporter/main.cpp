#include <iostream>
#include <memory>
#include <vector>
#include <regex>
#include <sstream>
#include <fstream>

#include <glad/glad.h>

#include "Common/Log.h"
#include "StaticMeshResources.h"
#include "Material.h"
#include "RHIDefinitions.h"
#include "StaticMeshVertexBuffer.h"
#include "Texture.h"
#include "VertexFactory.h"

#include <half/half.hpp>

using namespace nilou;
namespace fs = std::filesystem;

int main()
{
    // auto image = GetAssetLoader()->SyncOpenAndReadImage(R"(D:\Nilou\Assets\01_karelia.png)");
    // std::vector<half_float::half> half;
    // for (int i = 0; i < image->Width*image->Height*image->Channel; i++)
    // {
    //     half.push_back(half_float::half(image->data[i] / 4.0));
    // }
    // image->PixelFormat = EPixelFormat::PF_R16F;
    // image->data_size *= 2;
    // delete[] image->data;
    // image->data = new uint8[image->data_size];
    // memcpy(image->data, half.data(), image->data_size);
    // int NumMips = 1;//std::min(std::log2(image->Width), std::log2(image->Height));
    // std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>(NumMips, image);
    // Texture->SetSamplerParams(RHITextureParams::DefaultParams);
    // std::shared_ptr<UTexture> texture = std::make_shared<UTexture>(
    //     "karelia", std::move(Texture));
    // FArchive Ar;
    // texture->Serialize(Ar);
    // std::ofstream out{R"(D:\Nilou\Assets\01_karelia.json)"};
    // std::string s = Ar.json.dump();
    // out << s;
    // auto image = GetAssetLoader()->SyncOpenAndReadImage(R"(D:\Nilou\Assets\perlin_noise.dds)");
    // int NumMips = std::min(std::log2(image->Width), std::log2(image->Height));
    // std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>(NumMips, image);
    // Texture->SetSamplerParams(RHITextureParams::DefaultParams);
    // std::shared_ptr<UTexture> texture = std::make_shared<UTexture>(
    //     "PerlinNoise", std::move(Texture));
    // FArchive Ar;
    // texture->Serialize(Ar);
    // Ar.WriteToPath(R"(D:\Nilou\Content\Textures\PerlinNoiseTexture.nasset)");

    auto image = GetAssetLoader()->SyncOpenAndReadImage(R"(D:\Nilou\Assets\DEM.tif)");
    int NumMips = log2(image->Height/256)+1;
    std::vector<half_float::half> half;
    size_t mip0_size = 16384*16384*2;
    size_t all_mip_size = mip0_size * (1-pow(0.25, NumMips)) / (1-0.25);
    std::shared_ptr<FImage> new_img = std::make_shared<FImage>();
    new_img->data = new uint8[all_mip_size];
    new_img->data_size = all_mip_size;
    new_img->Channel = 1;
    new_img->Height = 16384;
    new_img->Width = 16384;
    new_img->PixelFormat = EPixelFormat::PF_R16F;
    for (int m = 0; m < NumMips; m++)
    {
        int mip_width = new_img->Width / glm::pow(2, m);
        int mip_height = new_img->Height / glm::pow(2, m);
        for (int row = 0; row < mip_height; row++)
        {
            for (int col = 0; col < mip_width; col++)
            {
                if (m == 0)
                {
                    auto dest = static_cast<half_float::half*>(new_img->Get(row, col, 0, m));
                    auto src = *static_cast<float*>(image->Get(row, col, 0, m));
                    *dest = half_float::half(src / 30);
                }
                else 
                {
                    auto dest = static_cast<half_float::half*>(new_img->Get(row, col, 0, m));
                    auto src1 = float(*static_cast<half_float::half*>(new_img->Get(row*2, col*2, 0, m-1)));
                    auto src2 = float(*static_cast<half_float::half*>(new_img->Get(row*2+1, col*2, 0, m-1)));
                    auto src3 = float(*static_cast<half_float::half*>(new_img->Get(row*2, col*2+1, 0, m-1)));
                    auto src4 = float(*static_cast<half_float::half*>(new_img->Get(row*2+1, col*2+1, 0, m-1)));
                    float dest_f = float(src1) + float(src2) + float(src3) + float(src4);
                    dest_f /= 4.f;
                    *dest = dest_f;
                }
            }
        }
    }
    std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>(NumMips, new_img);
    Texture->SetSamplerParams(RHITextureParams::DefaultParams);
    std::shared_ptr<UVirtualTexture> texture = std::make_shared<UTexture>(
        "TestVirtualHeightfield", std::move(Texture))->MakeVirtualTexture();
    FArchive Ar;
    texture->Serialize(Ar);
    Ar.WriteToPath(R"(D:\Nilou\Content\Textures\TestVirtualHeightfield.nasset)");
}