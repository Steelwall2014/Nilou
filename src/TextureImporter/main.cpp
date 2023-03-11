#include <iostream>
#include <memory>
#include <vector>
#include <regex>
#include <sstream>
#include <fstream>

#include <glad/glad.h>

#include "Common/Log.h"
#include "Common/StaticMeshResources.h"
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
    auto image = GetAssetLoader()->SyncOpenAndReadImage(R"(D:\Nilou\Assets\perlin_noise.dds)");
    int NumMips = std::min(std::log2(image->Width), std::log2(image->Height));
    std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>(NumMips, image);
    Texture->SetSamplerParams(RHITextureParams::DefaultParams);
    std::shared_ptr<UTexture> texture = std::make_shared<UTexture>(
        "PerlinNoise", std::move(Texture));
    FArchive Ar;
    texture->Serialize(Ar);
    Ar.WriteToPath(R"(D:\Nilou\Content\Textures\PerlinNoiseTexture.nasset)");
}