//#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#define STBI_MSC_SECURE_CRT
#include <iostream>
#include <memory>
#include <vector>
#include <regex>
#include <format>
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
// #include "Common/SceneNode.h"

#include "GLTFParser.h"

namespace nilou {
    GLTFParser *g_pGLTFParser = new GLTFParser;
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
    // std::shared_ptr<SceneObjectTexture> GLTFParser::ConvertGLTFTextureToSceneObject(tinygltf::Texture &gltf_texture)
    // {
    //     tinygltf::Image gltf_image = m_pTempModel->images[gltf_texture.source];
    //     std::shared_ptr<Image> image = std::make_shared<Image>();
    //     image->Width = gltf_image.width;
    //     image->Height = gltf_image.height;
    //     image->Channel = gltf_image.component;
    //     image->data_size = gltf_image.image.size();
    //     image->data = new unsigned char[image->data_size];
    //     image->type = gltf_image.pixel_type;


    //     memcpy(image->data, gltf_image.image.data(), image->data_size);
    //     std::shared_ptr<SceneObjectTexture> texture = std::make_shared<SceneObjectTexture>(image);
    //     if (gltf_texture.sampler != -1)
    //     {
    //         auto sampler = m_pTempModel->samplers[gltf_texture.sampler];

    //         if (sampler.minFilter != -1)
    //             texture->Min_Filter = GLTFFilterToETextureFilters(sampler.minFilter);
    //         if (sampler.magFilter != -1)
    //             texture->Mag_Filter = GLTFFilterToETextureFilters(sampler.magFilter);
    //         texture->Wrap_S = GLTFFilterToETextureWrapModes(sampler.wrapS);
    //         texture->Wrap_T = GLTFFilterToETextureWrapModes(sampler.wrapT);
    //     }
    //     return texture;
    // }

    // std::shared_ptr<SceneObjectMaterial> GLTFParser::ConvertGLTFMaterialToSceneObject(tinygltf::Material &gltf_material)
    // {
    //     std::shared_ptr<SceneObjectMaterial> material = std::make_shared<SceneObjectMaterial>();

    //     if (gltf_material.normalTexture.index != -1)
    //     {
    //         auto normal_texture = m_textures[gltf_material.normalTexture.index];
    //         material->SetNormal(normal_texture);
    //     }
    //     if (gltf_material.occlusionTexture.index != -1)
    //     {
    //         auto occlusion_texture = m_textures[gltf_material.occlusionTexture.index];
    //         material->SetOcclusion(occlusion_texture);
    //     }
    //     if (gltf_material.emissiveTexture.index != -1)
    //     {
    //         auto emissive_texture = m_textures[gltf_material.emissiveTexture.index];
    //         material->SetEmissive(emissive_texture);
    //     }
    //     if (gltf_material.pbrMetallicRoughness.baseColorTexture.index != -1)
    //     {
    //         auto baseColor_texture = m_textures[gltf_material.pbrMetallicRoughness.baseColorTexture.index];
    //         material->SetBaseColor(baseColor_texture);
    //     }
    //     if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
    //     {
    //         auto metallicRoughness_texture = m_textures[gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index];
    //         material->SetRoughnessMetallic(metallicRoughness_texture);
    //     }

    //     return material;
    // }

    // std::shared_ptr<BaseSceneNode> GLTFParser::ConvertGLTFNodeToSceneNode(tinygltf::Node &gltf_node)
    // {
    //     SceneObjectTransform trans;
    //     if (!gltf_node.matrix.empty())
    //     {
    //         if (gltf_node.matrix.size() != 16)
    //         {
    //             std::cerr << "Node transform matrix size invalid" << std::endl;
    //         }
    //         else
    //         {
    //             trans = SceneObjectTransform(glm::mat4(
    //                 gltf_node.matrix[0], gltf_node.matrix[1], gltf_node.matrix[2], gltf_node.matrix[3],
    //                 gltf_node.matrix[4], gltf_node.matrix[5], gltf_node.matrix[6], gltf_node.matrix[7],
    //                 gltf_node.matrix[8], gltf_node.matrix[9], gltf_node.matrix[10], gltf_node.matrix[11],
    //                 gltf_node.matrix[12], gltf_node.matrix[13], gltf_node.matrix[14], gltf_node.matrix[15]));
    //             //scene_node->SetRelativeTransform(trans);
    //         }
    //     }
    //     else
    //     {
    //         if (!gltf_node.scale.empty())
    //         {
    //             if (gltf_node.scale.size() != 3)
    //             {
    //                 std::cerr << "Node scale size invalid" << std::endl;
    //             }
    //             else
    //             {
    //                 trans.SetScale3D(glm::vec3(gltf_node.scale[0], gltf_node.scale[1], gltf_node.scale[2]));
    //             }
    //         }

    //         if (!gltf_node.translation.empty())
    //         {
    //             if (gltf_node.translation.size() != 3)
    //             {
    //                 std::cerr << "Node translation size invalid" << std::endl;
    //             }
    //             else
    //             {
    //                 trans.SetTranslation(glm::vec3(gltf_node.translation[0], gltf_node.translation[1], gltf_node.translation[2]));
    //             }
    //         }

    //         if (!gltf_node.rotation.empty())
    //         {
    //             if (gltf_node.rotation.size() != 4)
    //             {
    //                 std::cerr << "Node rotation size invalid" << std::endl;
    //             }
    //             else
    //             {
    //                 trans.SetRotation(glm::quat(gltf_node.rotation[0], gltf_node.rotation[1], gltf_node.rotation[2], gltf_node.rotation[3]));
    //             }
    //         }

    //         //scene_node->SetRelativeTransform(trans);
    //     }

    //     if (gltf_node.mesh != -1)
    //     {
    //         auto scene_node = std::make_shared<SceneGeometryNode>();
    //         auto scene_node_geom_obj = std::make_shared<SceneObjectGeometry>();
    //         auto scene_node_mesh_obj = std::make_shared<SceneObjectMesh>();

    //         scene_node->SetRelativeTransform(trans);

    //         tinygltf::Mesh &mesh = m_pTempModel->meshes[gltf_node.mesh];
    //         for (tinygltf::Primitive &gltf_prim : mesh.primitives)
    //         {
    //             SceneObjectMesh::Primitive prim;
    //             if (gltf_prim.material != -1)
    //                 prim.Material = ConvertGLTFMaterialToSceneObject(m_pTempModel->materials[gltf_prim.material]);

    //             auto &indices_accessor = m_pTempModel->accessors[gltf_prim.indices];
    //             auto &indices_bufferview = m_pTempModel->bufferViews[indices_accessor.bufferView];
    //             auto &indices_buffer = m_pTempModel->buffers[indices_bufferview.buffer];
    //             prim.pIndexArray = new unsigned char[indices_bufferview.byteLength];
    //             prim.IndexArraySize = indices_bufferview.byteLength;
    //             prim.count = indices_accessor.count;
    //             prim.type = indices_accessor.componentType;
    //             prim.mode = gltf_prim.mode;
    //             memcpy(prim.pIndexArray, indices_buffer.data.data()+ indices_bufferview.byteOffset, indices_bufferview.byteLength);

    //             for (auto &p : gltf_prim.attributes)
    //             {
    //                 SceneObjectMesh::AttriPointer attr_pointer;
    //                 auto &attr_accessor = m_pTempModel->accessors[p.second];
    //                 auto &attr_bufferview = m_pTempModel->bufferViews[attr_accessor.bufferView];
    //                 auto &attr_buffer = m_pTempModel->buffers[attr_bufferview.buffer];
    //                 attr_pointer.name = p.first;
    //                 attr_pointer.normalized = attr_accessor.normalized;
    //                 attr_pointer.offset = attr_accessor.byteOffset;
    //                 switch (attr_accessor.type)
    //                 {
    //                 case TINYGLTF_TYPE_SCALAR:
    //                     attr_pointer.size = 1;
    //                     break;
    //                 case TINYGLTF_TYPE_VEC2:
    //                     attr_pointer.size = 2;
    //                     break;
    //                 case TINYGLTF_TYPE_VEC3:
    //                     attr_pointer.size = 3;
    //                     break;
    //                 case TINYGLTF_TYPE_VEC4:
    //                     attr_pointer.size = 4;
    //                     break;
    //                 case TINYGLTF_TYPE_MAT2:
    //                     attr_pointer.size = 4;
    //                     break;
    //                 case TINYGLTF_TYPE_MAT3:
    //                     attr_pointer.size = 9;
    //                     break;
    //                 case TINYGLTF_TYPE_MAT4:
    //                     attr_pointer.size = 16;
    //                     break;
    //                 default:
    //                     attr_pointer.size = 0;
    //                     break;
    //                 }
    //                 attr_pointer.stride = attr_bufferview.byteStride;
    //                 attr_pointer.type = attr_accessor.componentType;
    //                 attr_pointer.pVertexAttriArray = new unsigned char[attr_bufferview.byteLength];
    //                 memcpy(attr_pointer.pVertexAttriArray, attr_buffer.data.data() + attr_bufferview.byteOffset, attr_bufferview.byteLength);
    //                 attr_pointer.VertexAttriArraySize = attr_bufferview.byteLength;

    //                 prim.AddAttribute(attr_pointer);
    //             }

    //             scene_node_mesh_obj->AddPrimitive(prim);
    //         }

    //         scene_node_geom_obj->AddMesh(scene_node_mesh_obj);
    //         scene_node->AddSceneObjectRef(scene_node_geom_obj);

    //         m_pTempScene->Geometries.push_back(scene_node_geom_obj);
    //         m_pTempScene->GeometryNodes.push_back(scene_node);
    //         return scene_node;
    //     }
    //     else
    //     {
    //         auto scene_node = std::make_shared<SceneEmptyNode>();
    //         scene_node->SetRelativeTransform(trans);
    //         return scene_node;
    //     }



    //     return nullptr;
    // }
    // void GLTFParser::build_scene_graph_recursive(std::vector<int> &children, BaseSceneNode &root_node)
    // {
    //     for (int child_index : children)
    //     {
    //         tinygltf::Node &gltf_child_node = m_pTempModel->nodes[child_index];
    //         auto child_node = ConvertGLTFNodeToSceneNode(gltf_child_node);
    //         root_node.AppendChild(child_node);
    //         build_scene_graph_recursive(gltf_child_node.children, *child_node);
    //     }
    // }
    // std::unique_ptr<Scene> GLTFParser::Parse(tinygltf::Model &model)
    // {
    //     m_pTempModel = &model;
    //     m_pTempScene = new Scene;
    //     for (auto &&gltf_texture : m_pTempModel->textures)
    //     {
    //         m_textures.push_back(ConvertGLTFTextureToSceneObject(gltf_texture));
    //     }
    //     build_scene_graph_recursive(m_pTempModel->scenes[0].nodes, *m_pTempScene->SceneGraph);
    //     //for (int root_index : model.scenes[0].nodes)
    //     //{
    //     //    tinygltf::Node &gltf_root_node = model.nodes[root_index];
    //     //    auto node = std::unique_ptr<BaseSceneNode>(ConvertGLTFNodeToSceneNode(gltf_root_node));
    //     //    build_scene_graph_recursive(model.nodes, gltf_root_node, *node);
    //     //    scene->SceneGraph->AppendChild(std::move(node));
    //     //}
    //     std::unique_ptr<Scene> res = std::unique_ptr<Scene>(m_pTempScene);
    //     m_pTempModel = nullptr;
    //     m_pTempScene = nullptr;
    //     return res;
    // }
    
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

	std::vector<std::shared_ptr<UStaticMesh>> GLTFParser::ParseToStaticMeshes(tinygltf::Model &model)
    {
        std::vector<std::shared_ptr<UStaticMesh>> StaticMeshes;
        std::vector<std::shared_ptr<FMaterial>> Materials = ParseToMaterials(model);
        for (auto &gltf_mesh : model.meshes)
        {
            for (int prim_index = 0; prim_index < gltf_mesh.primitives.size(); prim_index++)
            {
                tinygltf::Primitive &gltf_prim = gltf_mesh.primitives[prim_index];
                std::shared_ptr<UStaticMesh> StaticMesh = std::make_shared<UStaticMesh>(gltf_mesh.name+"_"+std::to_string(prim_index));
                FContentManager::GetContentManager().AddGlobalStaticMesh(StaticMesh->MeshName, StaticMesh);
                FStaticVertexFactory::FDataType Data;
                std::shared_ptr<FStaticMeshLODResources> Resource = std::make_shared<FStaticMeshLODResources>();

                {   // Material
                    if (gltf_prim.material != -1)
                        StaticMesh->StaticMaterial = Materials[gltf_prim.material].get();
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
                            Resource->IndexBuffer.Init(BufferToVector<uint8>(pos, indices_accessor.count, indices_bufferview.byteStride));
                        }
                        else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) 
                        {
                            Resource->IndexBuffer.Init(BufferToVector<uint16>(pos, indices_accessor.count, indices_bufferview.byteStride));
                        }
                        else if (indices_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) 
                        {
                            Resource->IndexBuffer.Init(BufferToVector<uint32>(pos, indices_accessor.count, indices_bufferview.byteStride));
                        }
                    }
                }

                bool HaveTexCoords = false;
                {   // Vertex
                    for (auto [attr_name, attr_index] : gltf_prim.attributes)
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
                                Resource->VertexBuffers.Positions.Init(BufferToVector<glm::vec3>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Resource->VertexBuffers.Positions.BindToVertexFactoryData(Data.PositionComponent);
                                StaticMesh->LocalBoundingBox.Min = vec3(attr_accessor.minValues[0], attr_accessor.minValues[1], attr_accessor.minValues[2]);
                                StaticMesh->LocalBoundingBox.Max = vec3(attr_accessor.maxValues[0], attr_accessor.maxValues[1], attr_accessor.maxValues[2]);
                            }
                        }
                        else if (attr_name == "NORMAL")
                        {
                            if (attr_accessor.type == TINYGLTF_TYPE_VEC3 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                                Resource->VertexBuffers.Normals.Init(BufferToVector<glm::vec3>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Resource->VertexBuffers.Normals.BindToVertexFactoryData(Data.NormalComponent);
                                //  = FVertexStreamComponent(
                                //     &Resource->VertexBuffers.Normals, 
                                //     0, 
                                //     Resource->VertexBuffers.Normals.GetStride(), 
                                //     EVertexElementType::VET_Float3);
                            }
                        }
                        else if (attr_name == "TANGENT")
                        {
                            if (attr_accessor.type == TINYGLTF_TYPE_VEC4 && attr_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                            {
                                uint8 *pos = attr_buffer.data.data() + attr_bufferview.byteOffset + attr_accessor.byteOffset;
                                Resource->VertexBuffers.Tangents.Init(BufferToVector<glm::vec4>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Resource->VertexBuffers.Tangents.BindToVertexFactoryData(Data.TangentComponent);
                                // Data.TangentComponent = FVertexStreamComponent(
                                //     &Resource->VertexBuffers.Tangents, 
                                //     0, 
                                //     Resource->VertexBuffers.Tangents.GetStride(), 
                                //     EVertexElementType::VET_Float4);
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
                                Resource->VertexBuffers.TexCoords[texcoord_index].Init(BufferToVector<glm::vec2>(pos, attr_accessor.count, attr_bufferview.byteStride));
                                Resource->VertexBuffers.TexCoords[texcoord_index].BindToVertexFactoryData(Data.TexCoordComponent[texcoord_index]);
                                // Data.TexCoordComponent = FVertexStreamComponent(
                                //     &Resource->VertexBuffers.TexCoords, 
                                //     0, 
                                //     Resource->VertexBuffers.TexCoords.GetStride(), 
                                //     EVertexElementType::VET_Float2);
                            }
                        }
                        else 
                        {
                            std::cout << "[ERROR] Attribute not supported: " << attr_name << std::endl;
                        }
                    }
                }
                std::shared_ptr<FStaticVertexFactory> VertexFactory;
                if (Data.PositionComponent.VertexBuffer && 
                    Data.NormalComponent.VertexBuffer && 
                    Data.TangentComponent.VertexBuffer && 
                    HaveTexCoords)
                {
                    VertexFactory = std::make_shared<FStaticVertexFactory>("FStaticVertexFactory");
                }
                else 
                {
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

                StaticMeshes.push_back(StaticMesh);
                {
                    Resource->InitResources();
                    StaticMesh->RenderData = std::make_unique<FStaticMeshRenderData>();
                    StaticMesh->RenderData->LODResources.push_back(Resource);
                    StaticMesh->RenderData->VertexFactory = VertexFactory;
                    VertexFactory->SetData(Data);
                }

            }
        }
        return StaticMeshes;
    }

    // EPixelFormat TranslateToEPixelFormat(int channel, int bits, int pixel_type)
    // {
    //     EPixelFormat PixelFormat = EPixelFormat::PF_UNKNOWN;
    //     if (channel == 1)
    //     {
    //         switch (pixel_type)
    //         {
    //             case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8; break;
    //             case TINYGLTF_COMPONENT_TYPE_FLOAT: 
    //                 if (bits == 16)
    //                     PixelFormat = EPixelFormat::PF_R16F;
    //                 else if (bits == 32)
    //                     PixelFormat = EPixelFormat::PF_R32F;
    //                 break;
    //             default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
    //         }
    //     }
    //     else if (channel == 2)
    //     {
    //         switch (pixel_type)
    //         {
    //             case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8; break;
    //             case TINYGLTF_COMPONENT_TYPE_FLOAT: 
    //                 if (bits == 16)
    //                     PixelFormat = EPixelFormat::PF_R16G16F;
    //                 else if (bits == 32)
    //                     PixelFormat = EPixelFormat::PF_R32G32F;
    //                 break;
    //             default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
    //         }
    //     }
    //     else if (channel == 3)
    //     {
    //         switch (pixel_type)
    //         {
    //             case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8B8; break;
    //             default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
    //         }
    //     }
    //     else if (channel == 4)
    //     {
    //         switch (pixel_type)
    //         {
    //             case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8B8A8; break;
    //             case TINYGLTF_COMPONENT_TYPE_FLOAT:
    //                 if (bits == 16)
    //                     PixelFormat = EPixelFormat::PF_R16G16B16A16F;
    //                 else if (bits == 32)
    //                     PixelFormat = EPixelFormat::PF_R32G32B32A32F;
    //                 break;
    //             default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
    //         }
    //     }
    //     else 
    //     {
    //         std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl;
    //     }
    //     return PixelFormat;
    // }

	std::vector<std::shared_ptr<FMaterial>> GLTFParser::ParseToMaterials(tinygltf::Model &model)
    {
        std::vector<std::shared_ptr<FMaterial>> Materials;
        std::vector<std::shared_ptr<FTexture>> Textures;
        for (tinygltf::Texture &gltf_texture : model.textures)
        {
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

            std::shared_ptr<FTexture> Texture = std::make_shared<FTexture>(gltf_texture.name, NumMips, image);
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
            Textures.push_back(Texture);
            FContentManager::GetContentManager().AddGlobalTexture(Texture->GetTextureName(), Texture);
        }

        for (tinygltf::Material &gltf_material : model.materials)
        {
            FRasterizerStateInitializer RasterizerState;
            if (gltf_material.doubleSided)
                RasterizerState.CullMode = CM_None;

            std::shared_ptr<FMaterial> Material = std::make_shared<FMaterial>(gltf_material.name, RasterizerState, FDepthStencilStateInitializer(), FBlendStateInitializer());
            FContentManager::GetContentManager().AddGlobalMaterial(Material->GetMaterialName(), Material);

            

            std::stringstream material_code;
            material_code << "#include \"../include/BasePassCommon.glsl\"\n";

            auto AccessTextures = [
                &Textures, &material_code, 
                &gltf_material, Material](int index, const std::string &sampler_name, const std::string &definition) {
                if (index != -1)
                {
                    Material->Textures[sampler_name] = Textures[index].get();
                    material_code << "uniform sampler " << sampler_name << ";\n";
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
            Materials.push_back(Material);
            // FDefaultMaterial Material;
            // FGLTFMaterial Material(RasterizerState, FDepthStencilStateInitializer());
            // Material.NormalTexture = AccessFTexture(gltf_material.normalTexture.index);
            // Material.OcclusionTexture = AccessFTexture(gltf_material.occlusionTexture.index);
            // Material.EmissiveTexture = AccessFTexture(gltf_material.emissiveTexture.index);
            // Material.BaseColorTexture = AccessFTexture(gltf_material.pbrMetallicRoughness.baseColorTexture.index);
            // Material.MetallicRoughnessTexture = AccessFTexture(gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index);
        }
        return Materials;
    }
}

