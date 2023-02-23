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

using namespace nilou;


static ETextureFilters GLTFFilterToETextureFilters(uint32 GLTFFilter)
{
    ETextureFilters filter;
    switch (GLTFFilter) {
        case TINYGLTF_TEXTURE_FILTER_NEAREST: filter = ETextureFilters::TF_Nearest; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR: filter = ETextureFilters::TF_Linear; break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST: filter = ETextureFilters::TF_Nearest_Mipmap_Nearest; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: filter = ETextureFilters::TF_Linear_Mipmap_Nearest; break;
        case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: filter = ETextureFilters::TF_Nearest_Mipmap_Linear; break;
        case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: filter = ETextureFilters::TF_Linear_Mipmap_Linear; break;
    }
    return filter;
}
static ETextureWrapModes GLTFFilterToETextureWrapModes(uint32 GLTFWrapMode)
{
    ETextureWrapModes mode;
    switch (GLTFWrapMode) {
        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: mode = ETextureWrapModes::TW_Clamp; break;
        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: mode = ETextureWrapModes::TW_Mirrored_Repeat; break;
        case TINYGLTF_TEXTURE_WRAP_REPEAT: mode = ETextureWrapModes::TW_Repeat; break;
    }
    return mode;
}

template<class T>
static std::vector<T> BufferToVector(uint8 *pos, int count, size_t stride)
{
    stride = stride == 0 ? sizeof(T) : stride;
    std::vector<T> Vertices;
    for (int i = 0; i < count; i++)
    {
        T *data = reinterpret_cast<T*>(pos);
        Vertices.push_back(*data);
        pos = pos + stride;
    }
    return Vertices;
}
std::shared_ptr<UTexture> NoColorTexture;
std::shared_ptr<UTexture> NoMetallicRoughnessTexture;
std::shared_ptr<UTexture> NoEmissiveTexture;
std::shared_ptr<UTexture> NoNormalTexture;

static std::shared_ptr<UMaterial> CreateGLTFMaterial()
{
    auto Material = std::make_shared<UMaterial>("GLTFMaterial");
    RHITextureParams texParams;
    texParams.Mag_Filter = ETextureFilters::TF_Nearest;
    texParams.Min_Filter = ETextureFilters::TF_Nearest;
    texParams.Wrap_S = ETextureWrapModes::TW_Clamp;
    texParams.Wrap_T = ETextureWrapModes::TW_Clamp;
    std::shared_ptr<FImage> NoColorImg = std::make_shared<FImage>();
    NoColorImg->Width = 1; NoColorImg->Height = 1; NoColorImg->Channel = 4; NoColorImg->data_size = 4;
    NoColorImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoColorImg->data = new uint8[4];
    NoColorImg->data[0] = 255; NoColorImg->data[1] = 255; NoColorImg->data[2] = 255; NoColorImg->data[3] = 255;
    NoColorTexture = std::make_shared<UTexture>("NoColorTexture", 1, NoColorImg);
    NoColorTexture->GetResource()->SetSamplerParams(texParams);
    // BeginInitResource(NoColorTexture->GetResource());

    std::shared_ptr<FImage> NoMetallicRoughnessImg = std::make_shared<FImage>();
    NoMetallicRoughnessImg->Width = 1; NoMetallicRoughnessImg->Height = 1; NoMetallicRoughnessImg->Channel = 4; NoMetallicRoughnessImg->data_size = 4;
    NoMetallicRoughnessImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoMetallicRoughnessImg->data = new uint8[4];
    NoMetallicRoughnessImg->data[0] = 0; NoMetallicRoughnessImg->data[1] = 255; NoMetallicRoughnessImg->data[2] = 255; NoMetallicRoughnessImg->data[3] = 255;
    NoMetallicRoughnessTexture = std::make_shared<UTexture>("NoMetallicRoughnessTexture", 1, NoMetallicRoughnessImg);
    NoMetallicRoughnessTexture->GetResource()->SetSamplerParams(texParams);
    // BeginInitResource(NoMetallicRoughnessTexture->GetResource());

    std::shared_ptr<FImage> NoEmissiveImg = std::make_shared<FImage>();
    NoEmissiveImg->Width = 1; NoEmissiveImg->Height = 1; NoEmissiveImg->Channel = 4; NoEmissiveImg->data_size = 4;
    NoEmissiveImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoEmissiveImg->data = new uint8[4];
    NoEmissiveImg->data[0] = 0; NoEmissiveImg->data[1] = 0; NoEmissiveImg->data[2] = 0; NoEmissiveImg->data[3] = 255;
    NoEmissiveTexture = std::make_shared<UTexture>("NoEmissiveTexture", 1, NoEmissiveImg);
    NoEmissiveTexture->GetResource()->SetSamplerParams(texParams);
    // BeginInitResource(NoEmissiveTexture->GetResource());

    std::shared_ptr<FImage> NoNormalImg = std::make_shared<FImage>();
    NoNormalImg->Width = 1; NoNormalImg->Height = 1; NoNormalImg->Channel = 4; NoNormalImg->data_size = 4;
    NoNormalImg->PixelFormat = EPixelFormat::PF_R8G8B8A8; NoNormalImg->data = new uint8[4];
    NoNormalImg->data[0] = 127; NoNormalImg->data[1] = 127; NoNormalImg->data[2] = 255; NoNormalImg->data[3] = 255;
    NoNormalTexture = std::make_shared<UTexture>("NoNormalTexture", 1, NoNormalImg);
    NoNormalTexture->GetResource()->SetSamplerParams(texParams);
    // BeginInitResource(NoNormalTexture->GetResource());

    Material->SetParameterValue("baseColorTexture", NoColorTexture.get());
    Material->SetParameterValue("metallicRoughnessTexture", NoMetallicRoughnessTexture.get());
    Material->SetParameterValue("emissiveTexture", NoEmissiveTexture.get());
    Material->SetParameterValue("normalTexture", NoNormalTexture.get());
    return Material;
}

static void ParseToMaterials(tinygltf::Model &model, 
    std::vector<std::shared_ptr<UMaterial>> &OutMaterials, 
    std::vector<std::shared_ptr<UTexture>> &OutTextures)
{
    OutMaterials.clear();
    OutTextures.clear();
    for (int TextureIndex = 0; TextureIndex < model.textures.size(); TextureIndex++)
    {
        tinygltf::Texture &gltf_texture = model.textures[TextureIndex];
        tinygltf::Image gltf_image = model.images[gltf_texture.source];
        std::shared_ptr<FImage> image = std::make_shared<FImage>();
        image->Width = gltf_image.width;
        image->Height = gltf_image.height;
        image->Channel = gltf_image.component;
        image->data_size = gltf_image.image.size();
        image->data = new uint8[image->data_size];
        image->PixelFormat = TranslateToEPixelFormat(gltf_image.component, gltf_image.bits, gltf_image.pixel_type);
        memcpy(image->data, gltf_image.image.data(), image->data_size);

        int NumMips = std::min(std::log2(gltf_image.width), std::log2(gltf_image.height));

        std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>(NumMips, image);
        
        RHITextureParams TextureParams;
        if (gltf_texture.sampler != -1)
        {
            tinygltf::Sampler &sampler = model.samplers[gltf_texture.sampler];

            if (sampler.minFilter != -1)
                TextureParams.Min_Filter = GLTFFilterToETextureFilters(sampler.minFilter);
            if (sampler.magFilter != -1)
                TextureParams.Mag_Filter = GLTFFilterToETextureFilters(sampler.magFilter);
            TextureParams.Wrap_S = GLTFFilterToETextureWrapModes(sampler.wrapS);
            TextureParams.Wrap_T = GLTFFilterToETextureWrapModes(sampler.wrapT);
        }
        Texture->SetSamplerParams(TextureParams);
        
        std::shared_ptr<UTexture> texture = std::make_shared<UTexture>(
            std::to_string(TextureIndex) + "_" + gltf_texture.name, std::move(Texture));
        OutTextures.push_back(texture);
    }

    for (int MaterialIndex = 0; MaterialIndex < model.materials.size(); MaterialIndex++)
    {
        tinygltf::Material &gltf_material = model.materials[MaterialIndex];
        auto Material = CreateGLTFMaterial();
        if (gltf_material.doubleSided)
            Material->GetResource()->RasterizerState.CullMode = ERasterizerCullMode::CM_None;
        auto AccessTextures = 
            [&OutTextures, &Material](int index, const std::string &sampler_name) {
            if (index != -1)
            {
                Material->SetParameterValue(sampler_name, OutTextures[index].get());
            }
        };
        AccessTextures(gltf_material.pbrMetallicRoughness.baseColorTexture.index, "baseColorTexture");
        AccessTextures(gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index, "metallicRoughnessTexture");
        AccessTextures(gltf_material.emissiveTexture.index, "emissiveTexture");
        AccessTextures(gltf_material.normalTexture.index, "normalTexture");

        std::string defines;
        auto &baseColorFactor = gltf_material.pbrMetallicRoughness.baseColorFactor;
        defines += "#define baseColorFactor_r (" + std::to_string(baseColorFactor[0]) + ")\n";
        defines += "#define baseColorFactor_g (" + std::to_string(baseColorFactor[1]) + ")\n";
        defines += "#define baseColorFactor_b (" + std::to_string(baseColorFactor[2]) + ")\n";
        defines += "#define baseColorFactor_a (" + std::to_string(baseColorFactor[3]) + ")\n";

        auto &emissiveFactor = gltf_material.emissiveFactor;
        defines += "#define emissiveFactor_r (" + std::to_string(emissiveFactor[0]) + ")\n";
        defines += "#define emissiveFactor_g (" + std::to_string(emissiveFactor[1]) + ")\n";
        defines += "#define emissiveFactor_b (" + std::to_string(emissiveFactor[2]) + ")\n";

        defines += "#define metallicFactor (" + std::to_string(gltf_material.pbrMetallicRoughness.metallicFactor) + ")\n";
        defines += "#define roughnessFactor (" + std::to_string(gltf_material.pbrMetallicRoughness.roughnessFactor) + ")\n";

        Material->UpdateCode(
            defines+
        R"(
            #include "../include/BasePassCommon.glsl"

            uniform sampler2D baseColorTexture;
            uniform sampler2D metallicRoughnessTexture;
            uniform sampler2D emissiveTexture;
            uniform sampler2D normalTexture;

            vec4 MaterialGetBaseColor(VS_Out vs_out)
            {
                vec4 baseColorFactor = vec4(baseColorFactor_r, baseColorFactor_g, baseColorFactor_b, baseColorFactor_a);
                return texture(baseColorTexture, vs_out.TexCoords) * baseColorFactor;
            }
            vec3 MaterialGetEmissive(VS_Out vs_out)
            {
                vec3 emissiveFactor = vec3(emissiveFactor_r, emissiveFactor_g, emissiveFactor_b);
                return texture(emissiveTexture, vs_out.TexCoords).rgb * emissiveFactor;
            }
            vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
            {
                vec3 tangent_normal = texture(normalTexture, vs_out.TexCoords).rgb;
                tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
                return normalize(vs_out.TBN * tangent_normal);
            }
            float MaterialGetRoughness(VS_Out vs_out)
            {
                return texture(metallicRoughnessTexture, vs_out.TexCoords).g * roughnessFactor;
            }
            float MaterialGetMetallic(VS_Out vs_out)
            {
                return texture(metallicRoughnessTexture, vs_out.TexCoords).b * metallicFactor;
            }
            vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
            {
                return vec3(0);
            }
        )", false);
        OutMaterials.push_back(Material);
    }
}


struct ParseResult
{
    std::vector<std::shared_ptr<UStaticMesh>> StaticMeshes;
    std::vector<std::shared_ptr<UMaterial>> Materials;
    std::vector<std::shared_ptr<UTexture>> Textures;
};
ParseResult ParseToStaticMeshes(tinygltf::Model &model)
{
    ParseResult Result;
    std::vector<std::shared_ptr<UStaticMesh>> &StaticMeshes = Result.StaticMeshes;
    std::vector<std::shared_ptr<UMaterial>> &Materials = Result.Materials;
    std::vector<std::shared_ptr<UTexture>> &Textures = Result.Textures;
    ParseToMaterials(model, Materials, Textures);
    for (auto &gltf_mesh : model.meshes)
    {
        std::shared_ptr<UStaticMesh> StaticMesh = std::make_shared<UStaticMesh>(gltf_mesh.name);
        std::unique_ptr<FStaticMeshLODResources> Resource = std::make_unique<FStaticMeshLODResources>();
        for (int prim_index = 0; prim_index < gltf_mesh.primitives.size(); prim_index++)
        {
            tinygltf::Primitive &gltf_prim = gltf_mesh.primitives[prim_index];
            FStaticVertexFactory::FDataType Data;
            std::unique_ptr<FStaticMeshSection> Section = std::make_unique<FStaticMeshSection>();
            
            {   // Material
                if (gltf_prim.material != -1)
                {
                    Section->MaterialIndex = StaticMesh->MaterialSlots.size();
                    StaticMesh->MaterialSlots.push_back(Materials[gltf_prim.material].get());
                }
            }

            {   // Index
                tinygltf::Accessor &indices_accessor = model.accessors[gltf_prim.indices];
                tinygltf::BufferView &indices_bufferview = model.bufferViews[indices_accessor.bufferView];
                tinygltf::Buffer &indices_buffer = model.buffers[indices_bufferview.buffer];
                uint8 *pos = indices_buffer.data.data() + indices_bufferview.byteOffset + indices_accessor.byteOffset;

                if (indices_accessor.type == TINYGLTF_TYPE_SCALAR)
                {
                    if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                    {
                        Section->IndexBuffer.Init(BufferToVector<uint8>(pos, indices_accessor.count, indices_bufferview.byteStride));
                    }
                    else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) 
                    {
                        Section->IndexBuffer.Init(BufferToVector<uint16>(pos, indices_accessor.count, indices_bufferview.byteStride));
                    }
                    else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) 
                    {
                        Section->IndexBuffer.Init(BufferToVector<uint32>(pos, indices_accessor.count, indices_bufferview.byteStride));
                    }
                }
            }

            bool HaveTexCoords = false;
            {   // Vertex
                for (auto &[attr_name, attr_index] : gltf_prim.attributes)
                {
                    tinygltf::Accessor &attr_accessor = model.accessors[attr_index];
                    tinygltf::BufferView &attr_bufferview = model.bufferViews[attr_accessor.bufferView];
                    tinygltf::Buffer &attr_buffer = model.buffers[attr_bufferview.buffer];
                    std::regex re("TEXCOORD_([0-9]+)");
                    std::smatch match;
                    if (attr_name == "POSITION")
                    {
                        if (attr_accessor.type == TINYGLTF_TYPE_VEC3 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            Section->VertexBuffers.Positions.Init(BufferToVector<glm::vec3>(pos, attr_accessor.count, attr_bufferview.byteStride));
                            Section->VertexBuffers.Positions.BindToVertexFactoryData(Data.PositionComponent);
                            StaticMesh->LocalBoundingBox.Min = vec3(attr_accessor.minValues[0], attr_accessor.minValues[1], attr_accessor.minValues[2]);
                            StaticMesh->LocalBoundingBox.Max = vec3(attr_accessor.maxValues[0], attr_accessor.maxValues[1], attr_accessor.maxValues[2]);
                        }
                    }
                    else if (attr_name == "NORMAL")
                    {
                        if (attr_accessor.type == TINYGLTF_TYPE_VEC3 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            Section->VertexBuffers.Normals.Init(BufferToVector<glm::vec3>(pos, attr_accessor.count, attr_bufferview.byteStride));
                            Section->VertexBuffers.Normals.BindToVertexFactoryData(Data.NormalComponent);
                        }
                    }
                    else if (attr_name == "TANGENT")
                    {
                        if (attr_accessor.type == TINYGLTF_TYPE_VEC4 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            Section->VertexBuffers.Tangents.Init(BufferToVector<glm::vec4>(pos, attr_accessor.count, attr_bufferview.byteStride));
                            Section->VertexBuffers.Tangents.BindToVertexFactoryData(Data.TangentComponent);
                        }
                    }
                    else if (std::regex_match(attr_name, match, re))
                    {
                        int texcoord_index = std::stoi(match[1].str());
                        if (texcoord_index < 0 || texcoord_index >= MAX_STATIC_TEXCOORDS)
                        {
                            NILOU_LOG(Error, "Invalid texcoord name: " + attr_name);
                            continue;
                        }
                        if (attr_accessor.type == TINYGLTF_TYPE_VEC2 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            HaveTexCoords = true;
                            uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                            Section->VertexBuffers.TexCoords[texcoord_index].Init(BufferToVector<glm::vec2>(pos, attr_accessor.count, attr_bufferview.byteStride));
                            Section->VertexBuffers.TexCoords[texcoord_index].BindToVertexFactoryData(Data.TexCoordComponent[texcoord_index]);
                        }
                    }
                    else 
                    {
                        std::cout << "[ERROR] Attribute not supported: " << attr_name << std::endl;
                    }
                }
            }
            Section->VertexFactory.SetData(Data);
            Resource->Sections.push_back(std::move(Section));

        }
        StaticMeshes.push_back(StaticMesh);
        StaticMesh->RenderData = std::make_unique<FStaticMeshRenderData>();
        StaticMesh->RenderData->LODResources.push_back(std::move(Resource));
    }

    return Result;
}

int main()
{
    CreateGLTFMaterial();
    std::string in_gltf = R"(D:\Nilou\Assets\Models\WaterBottle.gltf)";
    std::string out_dir = "D:\\Nilou\\Content\\Testgltf";
    std::shared_ptr<tinygltf::Model> Model = GetAssetLoader()->SyncReadGLTFModel(in_gltf.c_str());
    ParseResult Mesh = ParseToStaticMeshes(*Model);
    for (auto Texture : Mesh.Textures)
    {
        nlohmann::json texture;
        Texture->Serialize(texture);
        std::filesystem::path out_path = std::filesystem::path(out_dir) / std::filesystem::path(Texture->Name + ".json");
        std::ofstream out{out_path.generic_string()};
        std::string s = texture.dump();
        out << s;
    }
    for (int i = 0; i < Mesh.Materials.size(); i++)
    {
        nlohmann::json material;
        Mesh.Materials[i]->Serialize(material);
        nlohmann::json &content = material["Content"];
        for (auto &[Name, Texture] : Mesh.Materials[i]->GetTextureBindings())
        {
            content["Textures"][Name] = Texture->Name + ".json";
        }
        std::filesystem::path out_path = std::filesystem::path(out_dir) / std::filesystem::path(Mesh.Materials[i]->Name + ".json");
        std::ofstream out{out_path.generic_string()};
        std::string s = material.dump();
        out << s;
    }
    for (int i = 0; i < Mesh.StaticMeshes.size(); i++)
    {
        nlohmann::json mesh;
        Mesh.StaticMeshes[i]->Serialize(mesh);
        nlohmann::json &content = mesh["Content"];
        for (int j = 0; j < Mesh.StaticMeshes[i]->MaterialSlots.size(); j++)
        {
            content["MaterialSlots"][j] = Mesh.StaticMeshes[i]->MaterialSlots[j]->Name + ".json";
        }
        std::filesystem::path out_path = std::filesystem::path(out_dir) / std::filesystem::path(Mesh.StaticMeshes[i]->Name + ".json");
        std::ofstream out{out_path.generic_string()};
        std::string s = mesh.dump();
        out << s;
    }
}