#include <iostream>
#include <memory>
#include <vector>
#include <regex>
#include <sstream>

#include <glad/glad.h>

#include "Common/Log.h"
#include "Gamestatics.h"
#include "Common/StaticMeshResources.h"
#include "Common/ContentManager.h"
#include "Material.h"
#include "RHIDefinitions.h"
#include "StaticMeshVertexBuffer.h"
#include "Texture.h"
#include "VertexFactory.h"


namespace nilou {

    ETextureFilters GLTFFilterToETextureFilters(uint32 GLTFFilter)
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
    ETextureWrapModes GLTFFilterToETextureWrapModes(uint32 GLTFWrapMode)
    {
        ETextureWrapModes mode;
        switch (GLTFWrapMode) {
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: mode = ETextureWrapModes::TW_Clamp; break;
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: mode = ETextureWrapModes::TW_Mirrored_Repeat; break;
            case TINYGLTF_TEXTURE_WRAP_REPEAT: mode = ETextureWrapModes::TW_Repeat; break;
        }
        return mode;
    }

    bool GameStatics::StartsWith(const std::string &str, const std::string &temp)
    {
        if (str.size() < temp.size())
            return false;
        int length = std::min(str.size(), temp.size());
        for (int i = 0; i < length; i++)
        {
            if (str[i] != temp[i])
                return false;
        }
        return true;
    }

    bool GameStatics::EndsWith(const std::string &str, const std::string &temp)
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

    void GameStatics::Trim(std::string &s)
    {
        if( !s.empty() )
        {
            if (s[0] == ' ')
            {
                s.erase(0, s.find_first_not_of(" "));
                s.erase(s.find_last_not_of(" ") + 1);
                s.erase(0, s.find_first_not_of("\t"));
                s.erase(s.find_last_not_of("\t") + 1);
            }
            else if (s[0] == '\t')
            {
                s.erase(0, s.find_first_not_of("\t"));
                s.erase(s.find_last_not_of("\t") + 1);
                s.erase(0, s.find_first_not_of(" "));
                s.erase(s.find_last_not_of(" ") + 1);
            }
        }
    }

    size_t find_first_not_delim(const std::string &s, char delim, size_t pos)
    {
        for (size_t i = pos; i < s.size(); i++)
            if (s[i] != delim)
                return i;
        return std::string::npos;
    }
    std::vector<std::string> GameStatics::Split(const std::string &s, char delim)
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

    template<class T>
    std::vector<T> BufferToVector(uint8 *pos, int count, size_t stride)
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

    static void ParseToMaterials(tinygltf::Model &model, std::vector<std::shared_ptr<FMaterial>> &OutMaterials, std::vector<std::shared_ptr<FTexture>> &OutTextures)
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

            std::shared_ptr<FTexture> Texture = std::make_shared<FTexture>(
                std::to_string(TextureIndex) + "_" + gltf_texture.name, NumMips, image);
            BeginInitResource(Texture.get());
            
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
            OutTextures.push_back(Texture);
            // FContentManager::GetContentManager().AddGlobalTexture(Texture->GetTextureName(), Texture);
        }

        for (int MaterialIndex = 0; MaterialIndex < model.textures.size(); MaterialIndex++)
        {
            tinygltf::Material &gltf_material = model.materials[MaterialIndex];
            FRasterizerStateInitializer RasterizerState;
            if (gltf_material.doubleSided)
                RasterizerState.CullMode = CM_None;

            std::shared_ptr<FMaterial> Material = std::make_shared<FMaterial>(
                std::to_string(MaterialIndex) + "_" + gltf_material.name, RasterizerState, FDepthStencilStateInitializer(), FBlendStateInitializer());
            // FContentManager::GetContentManager().AddGlobalMaterial(Material->GetMaterialName(), Material);

            std::stringstream material_code;
            material_code << "#include \"../include/BasePassCommon.glsl\"\n";

            auto AccessTextures = [
                &OutTextures, &material_code, 
                &gltf_material, Material](int index, const std::string &sampler_name, const std::string &definition) {
                if (index != -1)
                {
                    Material->SetParameterValue(sampler_name, OutTextures[index].get());
                    material_code << "uniform sampler2D " << sampler_name << ";\n";
                    material_code << "#define " << definition << " 1 \n";
                }
            };
            AccessTextures(gltf_material.pbrMetallicRoughness.baseColorTexture.index, "baseColorMap", "HAS_BASECOLOR");
            AccessTextures(gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index, "metallicRoughnessMap", "HAS_METALLICROUGHNESS");
            AccessTextures(gltf_material.emissiveTexture.index, "emissiveMap", "HAS_EMISSIVE");
            AccessTextures(gltf_material.occlusionTexture.index, "occlusionMap", "HAS_OCCLUSION");
            AccessTextures(gltf_material.normalTexture.index, "normalMap", "HAS_NORMAL");

            material_code << R"(
                vec4 MaterialGetBaseColor(VS_Out vs_out)
                {
                #if HAS_BASECOLOR
                    return texture(baseColorMap, vs_out.TexCoords);
                #else
                    return vec4(0, 0, 0, 1);
                #endif
                }

                vec3 MaterialGetEmissive(VS_Out vs_out)
                {
                #if HAS_EMISSIVE
                    return texture(emissiveMap, vs_out.TexCoords).rgb;
                #else
                    return vec3(0, 0, 0);
                #endif
                }

                vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
                {
                #if HAS_NORMAL
                    vec3 tangent_normal = texture(normalMap, vs_out.TexCoords).rgb;
                    tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
                    return normalize(vs_out.TBN * tangent_normal);
                #else
                    return normalize(vs_out.TBN * vec3(0, 0, 1));
                #endif
                }

                vec4 MaterialGetOcclusion(VS_Out vs_out)
                {
                #if HAS_OCCLUSION
                    return texture(occlusionMap, vs_out.TexCoords);
                #else
                    return vec4(0, 0, 0, 0);
                #endif
                }

                float MaterialGetRoughness(VS_Out vs_out)
                {
                #if HAS_METALLICROUGHNESS
                    return texture(metallicRoughnessMap, vs_out.TexCoords).g;
                #else
                    return 0.5;
                #endif
                }

                float MaterialGetMetallic(VS_Out vs_out)
                {
                #if HAS_METALLICROUGHNESS
                    return texture(metallicRoughnessMap, vs_out.TexCoords).b;
                #else
                    return 0.5;
                #endif
                }

                vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
                {
                    return vec3(0);
                }
            )";
            Material->UpdateMaterialCode(material_code.str());
            OutMaterials.push_back(Material);
        }
    }

	GLTFParseResult GameStatics::ParseToStaticMeshes(tinygltf::Model &model)
    {
        GLTFParseResult Result;
        std::vector<std::shared_ptr<UStaticMesh>> &StaticMeshes = Result.StaticMeshes;
        std::vector<std::shared_ptr<FMaterial>> &Materials = Result.Materials;
        std::vector<std::shared_ptr<FTexture>> &Textures = Result.Textures;
        ParseToMaterials(model, Materials, Textures);
        for (auto &gltf_mesh : model.meshes)
        {
            std::shared_ptr<UStaticMesh> StaticMesh = std::make_shared<UStaticMesh>(gltf_mesh.name);
            // FContentManager::GetContentManager().AddGlobalStaticMesh(StaticMesh->MeshName, StaticMesh);
            std::shared_ptr<FStaticMeshLODResources> Resource = std::make_shared<FStaticMeshLODResources>();
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
                if (Data.PositionComponent.VertexBuffer && 
                    Data.NormalComponent.VertexBuffer && 
                    Data.TangentComponent.VertexBuffer && 
                    HaveTexCoords)
                {
                    Section->VertexFactory.SetData(Data);
                }
                else 
                {
                    Section->VertexFactory.SetData(Data);
                    NILOU_LOG(Error, "Currently only gltf models with positions, normals and tangents are accepted")   
                }
                // if (Data.PositionComponent.VertexBuffer)
                // {
                //     if (Data.NormalComponent.VertexBuffer)
                //         if (Data.TangentComponent.VertexBuffer && HaveTexCoords)
                //             VertexFactory = std::make_shared<FStaticVertexFactory>("FStaticVertexFactory");
                //         else 
                //             VertexFactory = std::make_shared<FPositionAndNormalOnlyVertexFactory>("FPositionAndNormalOnlyVertexFactory");
                //     else 
                //         VertexFactory = std::make_shared<FPositionOnlyVertexFactory>("FPositionOnlyVertexFactory");
                // }
                Resource->Sections.push_back(std::move(Section));

            }
            StaticMeshes.push_back(StaticMesh);
            {
                Resource->InitResources();
                StaticMesh->RenderData = std::make_unique<FStaticMeshRenderData>();
                StaticMesh->RenderData->LODResources.push_back(Resource);
            }
        }
        return Result;
    }
}
