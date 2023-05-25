#include <set>

#include "StaticMeshResources.h"
#include "DynamicMeshResources.h"
#include "Common/ContentManager.h"
#include "Material.h"
#include "DynamicRHI.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include "Common/Path.h"


namespace nilou {

    namespace fs = std::filesystem;

    void FStaticMeshVertexBuffers::InitFromDynamicVertex(FStaticVertexFactory *VertexFactory, const std::vector<FDynamicMeshVertex> &Vertices)
    {
        Positions.Init(Vertices.size());
        Normals.Init(Vertices.size());
        Tangents.Init(Vertices.size());
        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
            TexCoords[i].Init(Vertices.size());
        Colors.Init(Vertices.size());
        
        for (int VertexIndex = 0; VertexIndex < Vertices.size(); VertexIndex++)
        {
            const FDynamicMeshVertex &Vertex = Vertices[VertexIndex];
            Positions.SetVertexValue(VertexIndex, Vertex.Position);
            Normals.SetVertexValue(VertexIndex, Vertex.Normal);
            Tangents.SetVertexValue(VertexIndex, Vertex.Tangent);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                TexCoords[i].SetVertexValue(VertexIndex, Vertex.TextureCoordinate[i]);
            Colors.SetVertexValue(VertexIndex, vec4(Vertex.Color, 1));
        }

        {
            BeginInitResource(&Positions);
            BeginInitResource(&Normals);
            BeginInitResource(&Tangents);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                BeginInitResource(&TexCoords[i]);
            BeginInitResource(&Colors);
            
            FStaticVertexFactory::FDataType Data;
            Positions.BindToVertexFactoryData(Data.PositionComponent);
            Normals.BindToVertexFactoryData(Data.NormalComponent);
            Tangents.BindToVertexFactoryData(Data.TangentComponent);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                TexCoords[i].BindToVertexFactoryData(Data.TexCoordComponent[i]);
            Colors.BindToVertexFactoryData(Data.ColorComponent);
            VertexFactory->SetData(Data);
            VertexFactory->InitVertexFactory();
        }
    }

    FRHIVertexInput AccessStreamComponent(const FVertexStreamComponent &Component, uint8 Location)
    {
        FRHIVertexInput VertexInput;
        VertexInput.VertexBuffer = Component.VertexBuffer->VertexBufferRHI.get();
        VertexInput.Location = Location;
        VertexInput.Offset = Component.Offset;
        VertexInput.Stride = Component.Stride;
        VertexInput.Type = Component.Type;
        return VertexInput;
    }

    void FStaticVertexFactory::SetData(const FDataType &InData)
    {
        Data = InData;
    }

    void FStaticVertexFactory::InitVertexFactory()
    {
        ENQUEUE_RENDER_COMMAND(FStaticVertexFactory_InitVertexFactory)(
            [this](FDynamicRHI*) 
            {
                VertexInputList.clear();
                if (Data.PositionComponent.VertexBuffer != nullptr)
                {
                    VertexInputList.push_back(AccessStreamComponent(Data.PositionComponent, 0));
                }
                if (Data.NormalComponent.VertexBuffer != nullptr)
                {
                    VertexInputList.push_back(AccessStreamComponent(Data.NormalComponent, 1));
                }
                if (Data.TangentComponent.VertexBuffer != nullptr)
                {
                    VertexInputList.push_back(AccessStreamComponent(Data.TangentComponent, 2));
                }
                if (Data.ColorComponent.VertexBuffer != nullptr)
                {
                    VertexInputList.push_back(AccessStreamComponent(Data.ColorComponent, 3));
                }
                for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                {
                    if (Data.TexCoordComponent[i].VertexBuffer != nullptr)
                    {
                        VertexInputList.push_back(AccessStreamComponent(Data.TexCoordComponent[i], 4+i));
                    }
                }
            });
    }

    // FRHIVertexInputList* FStaticVertexFactory::GetVertexInputList() const
    // {
    //     OutVertexInputs.clear();
    //     if (Data.PositionComponent.VertexBuffer != nullptr)
    //     {
    //         OutVertexInputs.push_back(AccessStreamComponent(Data.PositionComponent, 0));
    //     }
    //     if (Data.NormalComponent.VertexBuffer != nullptr)
    //     {
    //         OutVertexInputs.push_back(AccessStreamComponent(Data.NormalComponent, 1));
    //     }
    //     if (Data.TangentComponent.VertexBuffer != nullptr)
    //     {
    //         OutVertexInputs.push_back(AccessStreamComponent(Data.TangentComponent, 2));
    //     }
    //     if (Data.ColorComponent.VertexBuffer != nullptr)
    //     {
    //         OutVertexInputs.push_back(AccessStreamComponent(Data.ColorComponent, 3));
    //     }
    //     for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
    //     {
    //         if (Data.TexCoordComponent[i].VertexBuffer != nullptr)
    //         {
    //             OutVertexInputs.push_back(AccessStreamComponent(Data.TexCoordComponent[i], 4+i));
    //         }
    //     }
    //     return &OutVertexInputs;
    // }

    bool FStaticVertexFactory::ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters)
    {
        return true;
    }

    void FStaticVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) 
    { 
    }

    IMPLEMENT_VERTEX_FACTORY_TYPE(FStaticVertexFactory, "/Shaders/VertexFactories/StaticMeshVertexFactory.glsl")

    void FStaticMeshLODResources::InitResources()
    {
        for (int SectionIndex = 0; SectionIndex < Sections.size(); SectionIndex++)
        {
            FStaticMeshSection* Section = Sections[SectionIndex];
            if (Section->VertexBuffers.Colors.GetVertexData() != nullptr)
                BeginInitResource(&Section->VertexBuffers.Colors);
            if (Section->VertexBuffers.Positions.GetVertexData() != nullptr)
                BeginInitResource(&Section->VertexBuffers.Positions);
            if (Section->VertexBuffers.Normals.GetVertexData() != nullptr)
                BeginInitResource(&Section->VertexBuffers.Normals);
            if (Section->VertexBuffers.Tangents.GetVertexData() != nullptr)
                BeginInitResource(&Section->VertexBuffers.Tangents);
            if (Section->VertexBuffers.Colors.GetVertexData() != nullptr)
                BeginInitResource(&Section->VertexBuffers.Colors);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
            {
                if (Section->VertexBuffers.TexCoords[i].GetVertexData() != nullptr)
                    BeginInitResource(&Section->VertexBuffers.TexCoords[i]);
            }
            if (Section->IndexBuffer.GetIndiceData() != nullptr)
                BeginInitResource(&Section->IndexBuffer);
            Section->VertexFactory.InitVertexFactory();
        }
        bIsInitialized = true;
    }

    void FStaticMeshLODResources::ReleaseResources()
    {
        bIsInitialized = false;
        for (int SectionIndex = 0; SectionIndex < Sections.size(); SectionIndex++)
        {
            FStaticMeshSection &Section = *Sections[SectionIndex];
            if (Section.VertexBuffers.Colors.GetVertexData() != nullptr)
                BeginReleaseResource(&Section.VertexBuffers.Colors);
            if (Section.VertexBuffers.Positions.GetVertexData() != nullptr)
                BeginReleaseResource(&Section.VertexBuffers.Positions);
            if (Section.VertexBuffers.Normals.GetVertexData() != nullptr)
                BeginReleaseResource(&Section.VertexBuffers.Normals);
            if (Section.VertexBuffers.Tangents.GetVertexData() != nullptr)
                BeginReleaseResource(&Section.VertexBuffers.Tangents);
            if (Section.VertexBuffers.Colors.GetVertexData() != nullptr)
                BeginReleaseResource(&Section.VertexBuffers.Colors);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
            {
                if (Section.VertexBuffers.TexCoords[i].GetVertexData() != nullptr)
                    BeginReleaseResource(&Section.VertexBuffers.TexCoords[i]);
            }
            if (Section.IndexBuffer.GetIndiceData() != nullptr)
                BeginReleaseResource(&Section.IndexBuffer);
        }
    }

    int32 FStaticMeshSection::GetNumVertices() const
    {
        return VertexBuffers.Positions.GetNumVertices();
    }

    // int32 FStaticMeshLODResources::GetNumTexCoords() const
    // {
    //     return VertexBuffers.TexCoords[0].GetNumVertices();
    // }

    void UStaticMesh::ReleaseResources()
    {
        if (RenderData)
        {
            FStaticMeshRenderData* ToDelete = RenderData;
            ENQUEUE_RENDER_COMMAND(UStaticMesh_ReleaseResources)(
                [ToDelete](FDynamicRHI*) {
                    for (int i = 0; i < ToDelete->LODResources.size(); i++)
                    {
                        ToDelete->LODResources[i]->ReleaseResources();
                        delete ToDelete->LODResources[i];
                    }
                    delete ToDelete;
                });
            RenderData = nullptr;
        }
    }

    void UStaticMesh::PostDeserialize()
    {
        for (auto &lod_resource : LODResourcesData)
        {
            auto LODResouce = new FStaticMeshLODResources();
            for (auto &section : lod_resource.Sections)
            {
                auto Section = new FStaticMeshSection();
                Section->MaterialIndex = section.MaterialIndex;
                Section->bCastShadow = section.bCastShadow;
                {
                    int Stride = section.IndexBuffer.Stride;
                    int NumIndices = section.IndexBuffer.Data.BufferSize / Stride;
                    Section->IndexBuffer.Init(section.IndexBuffer.Data.Buffer.get(), NumIndices, Stride);
                }
                FStaticVertexFactory::FDataType VFData;
                auto load_data = 
                    [](FVertexIndexBufferData& BufferData, FVertexStreamComponent& Component, auto& Buffer) 
                    {
                        if (BufferData.Data.Buffer)
                        {
                            int Stride = BufferData.Stride;
                            int NumVertices = BufferData.Data.BufferSize / Stride;
                            Buffer.Init(BufferData.Data.Buffer.get(), Stride, NumVertices);
                            Buffer.BindToVertexFactoryData(Component);
                        }
                    };
                load_data(section.Positions, VFData.PositionComponent, Section->VertexBuffers.Positions);
                load_data(section.Normals, VFData.NormalComponent, Section->VertexBuffers.Normals);
                load_data(section.Colors, VFData.ColorComponent, Section->VertexBuffers.Colors);
                load_data(section.Tangents, VFData.TangentComponent, Section->VertexBuffers.Tangents);
                for (int i = 0; i < section.TexCoords.size(); i++)
                {
                    load_data(section.TexCoords[i], VFData.TexCoordComponent[i], Section->VertexBuffers.TexCoords[i]);
                }
                Section->VertexFactory.SetData(VFData);
                LODResouce->Sections.push_back(Section);
            }
            RenderData->LODResources.push_back(LODResouce);
        }    
        RenderData->InitResources();  
    }

    // void UStaticMesh::Serialize(FArchive &Ar)
    // {
    //     nlohmann::json &json = Ar.json;
    //     json["ClassName"] = "UStaticMesh";
    //     nlohmann::json &content = json["Content"];
    //     content["Name"] = Name;
    //     TStaticSerializer<FBoundingBox>::Serialize(LocalBoundingBox, content["LocalBoundingBox"], Ar.OutBuffers);
    //     TStaticSerializer<FStaticMeshRenderData>::Serialize(*RenderData, content["RenderData"], Ar.OutBuffers);
    //     for (int i = 0; i < MaterialSlots.size(); i++)
    //     {
    //         if (!MaterialSlots[i]->SerializationPath.empty())
    //         {
    //             content["MaterialSlots"][i] = MaterialSlots[i]->SerializationPath.generic_string();
    //         }
    //     }
    // }

    // static void FStaticMeshRenderDataDeserialize(FStaticMeshRenderData &Object, nlohmann::json &json, void* Buffer)
    // {
        
    //     nlohmann::json &content = json["Content"];

    //     nlohmann::json &lod_resources = content["LODResources"];

    //     for (auto &lod_resource : lod_resources)
    //     {
    //         auto LODResouce = new FStaticMeshLODResources();
    //         for (auto &section : lod_resource["Sections"])
    //         {
    //             auto Section = new FStaticMeshSection();

    //             if (section.contains("IndexBuffer"))
    //             {
    //                 int NumIndices = section["IndexBuffer"]["NumIndices"];
    //                 int Stride = section["IndexBuffer"]["Stride"];
    //                 int BufferOffset = section["IndexBuffer"]["Data"]["BufferOffset"];
    //                 // std::string Data = SerializeHelper::Base64Decode(section["IndexBuffer"]["Data"]);
    //                 Section->IndexBuffer.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, NumIndices, Stride);
    //             }
    //             FStaticVertexFactory::FDataType VFData;
    //             if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Positions"))
    //             {
    //                 int NumVertices = section["VertexBuffers"]["Positions"]["NumVertices"];
    //                 int Stride = section["VertexBuffers"]["Positions"]["Stride"];
    //                 int BufferOffset = section["VertexBuffers"]["Positions"]["Data"]["BufferOffset"];
    //                 // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Positions"]["Data"]);
    //                 Section->VertexBuffers.Positions.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
    //                 Section->VertexBuffers.Positions.BindToVertexFactoryData(VFData.PositionComponent);
    //             }
    //             if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Normals"))
    //             {
    //                 int NumVertices = section["VertexBuffers"]["Normals"]["NumVertices"];
    //                 int Stride = section["VertexBuffers"]["Normals"]["Stride"];
    //                 int BufferOffset = section["VertexBuffers"]["Normals"]["Data"]["BufferOffset"];
    //                 // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Normals"]["Data"]);
    //                 Section->VertexBuffers.Normals.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
    //                 Section->VertexBuffers.Normals.BindToVertexFactoryData(VFData.NormalComponent);
    //             }
    //             if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Tangents"))
    //             {
    //                 int NumVertices = section["VertexBuffers"]["Tangents"]["NumVertices"];
    //                 int Stride = section["VertexBuffers"]["Tangents"]["Stride"];
    //                 int BufferOffset = section["VertexBuffers"]["Tangents"]["Data"]["BufferOffset"];
    //                 // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Tangents"]["Data"]);
    //                 Section->VertexBuffers.Tangents.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
    //                 Section->VertexBuffers.Tangents.BindToVertexFactoryData(VFData.TangentComponent);
    //             }
    //             if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Colors"))
    //             {
    //                 int NumVertices = section["VertexBuffers"]["Colors"]["NumVertices"];
    //                 int Stride = section["VertexBuffers"]["Colors"]["Stride"];
    //                 int BufferOffset = section["VertexBuffers"]["Colors"]["Data"]["BufferOffset"];
    //                 // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Colors"]["Data"]);
    //                 Section->VertexBuffers.Colors.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
    //                 Section->VertexBuffers.Colors.BindToVertexFactoryData(VFData.ColorComponent);
    //             }
    //             if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("TexCoords"))
    //             {
    //                 for (int i = 0; i < section["VertexBuffers"]["TexCoords"].size(); i++)
    //                 {
    //                     int NumVertices = section["VertexBuffers"]["TexCoords"][i]["NumVertices"];
    //                     int Stride = section["VertexBuffers"]["TexCoords"][i]["Stride"];
    //                     int BufferOffset = section["VertexBuffers"]["TexCoords"][i]["Data"]["BufferOffset"];
    //                     // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["TexCoords"][i]["Data"]);
    //                     Section->VertexBuffers.TexCoords[i].Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
    //                     Section->VertexBuffers.TexCoords[i].BindToVertexFactoryData(VFData.TexCoordComponent[i]);
    //                 }
    //             }
    //             Section->VertexFactory.SetData(VFData);
    //             LODResouce->Sections.push_back(Section);
    //         }
    //         Object.LODResources.push_back(LODResouce);
    //     }
    // }

    // void UStaticMesh::MDeserialize(FArchive &Ar)
    // {
    //     nlohmann::json &json = Ar.Node;
    //     nlohmann::json &content = json["Content"];
    //     Name = content["Name"];
    //     {
    //         FArchive local_Ar(content["LocalBoundingBox"]["Content"], Ar);
    //         TStaticSerializer<FBoundingBox>::Deserialize(LocalBoundingBox, local_Ar);
    //     }
    //     {
    //         FStaticMeshRenderDataDeserialize(*RenderData, content["RenderData"], Ar.InBuffer.get());
    //     }
    //     RenderData->InitResources();
    //     for (int i = 0; i < content["MaterialSlots"].size(); i++)
    //     {
    //         fs::path material_path = content["MaterialSlots"][i].get<std::string>();
    //         UMaterial *Material = GetContentManager()->GetMaterialByPath(material_path);
    //         MaterialSlots.push_back(Material);
    //     }
    //     for (auto LODResource : RenderData->LODResources)
    //     {
    //         auto& LODResourceData = LODResourcesData.emplace_back();
    //         for (auto Section : LODResource->Sections)
    //         {
    //             auto& SectionData = LODResourceData.Sections.emplace_back();
    //             SectionData.bCastShadow = Section->bCastShadow;
    //             SectionData.MaterialIndex = Section->MaterialIndex;
    //             SectionData.IndexBuffer.Stride = Section->IndexBuffer.Stride;
    //             SectionData.IndexBuffer.Data.BufferSize = Section->IndexBuffer.NumIndices * Section->IndexBuffer.Stride;
    //             SectionData.IndexBuffer.Data.Buffer = std::make_shared<unsigned char[]>(SectionData.IndexBuffer.Data.BufferSize);
    //             std::memcpy(SectionData.IndexBuffer.Data.Buffer.get(), Section->IndexBuffer.GetIndiceData(), SectionData.IndexBuffer.Data.BufferSize);
                
    //             auto copy_data = [](FVertexIndexBufferData& Dest, auto& Src) {
    //                 Dest.Stride = Src.GetStride();
    //                 Dest.Data.BufferSize = Src.GetStride() * Src.GetNumVertices();
    //                 Dest.Data.Buffer = std::make_shared<unsigned char[]>(Dest.Data.BufferSize);
    //                 std::memcpy(Dest.Data.Buffer.get(), Src.GetVertexData(), Dest.Data.BufferSize);
    //             };
    //             copy_data(SectionData.Colors, Section->VertexBuffers.Colors);
    //             copy_data(SectionData.Normals, Section->VertexBuffers.Normals);
    //             copy_data(SectionData.Positions, Section->VertexBuffers.Positions);
    //             copy_data(SectionData.Tangents, Section->VertexBuffers.Tangents);
    //             copy_data(SectionData.TexCoords.emplace_back(), Section->VertexBuffers.TexCoords[0]);
    //         }
    //     }
    // }
    
}
