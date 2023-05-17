#pragma once
// #include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "UniformBuffer.h"
#include "Common/World.h"
#include "VertexFactory.h"
#include "Platform.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include "Shader.h"
#include "StaticMeshIndexBuffer.h"
#include "StaticMeshVertexBuffer.h"
#include "RHI.h"
#include "Templates/ObjectMacros.h"

namespace nilou {

    class FStaticVertexFactory : public FVertexFactory
    {
        DECLARE_VERTEX_FACTORY_TYPE(FStaticVertexFactory)
    public:

        FStaticVertexFactory() { }

        struct FDataType
        {
            FVertexStreamComponent PositionComponent;
            
            FVertexStreamComponent NormalComponent;
            
            FVertexStreamComponent TangentComponent;
            
            FVertexStreamComponent TexCoordComponent[MAX_STATIC_TEXCOORDS];

            FVertexStreamComponent ColorComponent;
        };

	    void SetData(const FDataType& InData);

        virtual std::vector<FRHIVertexInput> GetVertexInputList() const override;

        static bool ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters);

        static void ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment);
        
    protected:
        FDataType Data;

    };

    struct FStaticMeshVertexBuffers
    {
    public:
        FStaticMeshVertexBuffer<glm::vec3> Positions;
        
        FStaticMeshVertexBuffer<glm::vec3> Normals;
        
        FStaticMeshVertexBuffer<glm::vec4> Tangents;
        
        FStaticMeshVertexBuffer<glm::vec4> Colors;
        
        FStaticMeshVertexBuffer<glm::vec2> TexCoords[MAX_STATIC_TEXCOORDS];

        void InitFromDynamicVertex(FStaticVertexFactory *VertexFactory, const std::vector<class FDynamicMeshVertex> &Vertices);

        void ReleaseResource()
        {
            Positions.ReleaseResource();
            Normals.ReleaseResource();
            Tangents.ReleaseResource();
            Colors.ReleaseResource();
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                TexCoords[i].ReleaseResource();
        }
    };

    struct FStaticMeshSection
    {
        /** The index of the material with which to render this section. */
        int32 MaterialIndex;

        FStaticMeshVertexBuffers VertexBuffers;

        FStaticMeshIndexBuffer IndexBuffer;

        FStaticVertexFactory VertexFactory;

        /** If true, this section will cast a shadow. */
        bool bCastShadow = true;

        int32 GetNumVertices() const;
    };

    struct FStaticMeshLODResources
    {
    public:

        std::vector<FStaticMeshSection*> Sections;

        void InitResources();

        void ReleaseResources();

        // int32 GetNumTexCoords() const;

        bool IsInitialized() const { return bIsInitialized; }

    private:
        bool bIsInitialized = false;
    };

    class FStaticMeshRenderData
    {
    public:
        std::vector<FStaticMeshLODResources*> LODResources;

        ~FStaticMeshRenderData()
        {
            for (auto &Resource : LODResources)
            {
                Resource->ReleaseResources();
            }
        }

        bool IsInitialized() 
        { 
            for (auto &Resource : LODResources)
            {
                if (!Resource->IsInitialized())
                    return false;
            }
            return true;
        }
        
        void InitResources()
        {
            for (auto &Resource : LODResources)
            {
                Resource->InitResources();
            }
        }
    };

    class NCLASS UStaticMesh : public UObject
    {
        GENERATED_BODY()
    public:
        UStaticMesh() 
            : RenderData(new FStaticMeshRenderData())
        { }
        std::string Name;

        FStaticMeshRenderData* RenderData;

        std::vector<class UMaterial *> MaterialSlots;

        FBoundingBox LocalBoundingBox;

        virtual void Serialize(FArchive &Ar) override;

        virtual void Deserialize(FArchive &Ar) override;

        virtual ~UStaticMesh() { ReleaseResources(); }
        void ReleaseResources();
    };

    template<>
    class TStaticSerializer<FStaticMeshRenderData>
    {
    public:
        static void Serialize(const FStaticMeshRenderData &Object, nlohmann::json &json, FArchiveBuffers &Buffers)
        {
            json["ClassName"] = "FStaticMeshRenderData";
            nlohmann::json &content = json["Content"];
            for (int LODResourceIndex = 0; LODResourceIndex < Object.LODResources.size(); LODResourceIndex++)
            {
                auto &LODResource = Object.LODResources[LODResourceIndex];
                nlohmann::json &lod_resource = content["LODResources"][LODResourceIndex];
                for (int SectionIndex = 0; SectionIndex < LODResource->Sections.size(); SectionIndex++)
                {
                    auto &Section = LODResource->Sections[SectionIndex];
                    nlohmann::json &section = lod_resource["Sections"][SectionIndex];

                    if (void *Data = Section->IndexBuffer.GetIndiceData())
                    {
                        int NumIndices = Section->IndexBuffer.NumIndices;
                        int Stride = Section->IndexBuffer.Stride;
                        section["IndexBuffer"]["NumIndices"] = NumIndices;
                        section["IndexBuffer"]["Stride"] = Stride;
                        Buffers.AddBuffer(section["IndexBuffer"]["Data"], NumIndices*Stride, Data);
                            // SerializeHelper::Base64Encode(
                            //     (const unsigned char *)Data, 
                            //     NumIndices*Stride);
                    }
                    if (void *Data = Section->VertexBuffers.Positions.GetVertexData())
                    {
                        int NumVertices = Section->VertexBuffers.Positions.GetNumVertices();
                        int Stride = Section->VertexBuffers.Positions.GetStride();
                        section["VertexBuffers"]["Positions"]["NumVertices"] = NumVertices;
                        section["VertexBuffers"]["Positions"]["Stride"] = Stride;
                        Buffers.AddBuffer(
                            section["VertexBuffers"]["Positions"]["Data"], 
                            NumVertices*Stride, 
                            Section->VertexBuffers.Positions.GetVertexData());
                        // section["VertexBuffers"]["Positions"]["Data"] = SerializeHelper::Base64Encode(
                        //         (const unsigned char *)Section->VertexBuffers.Positions.GetVertexData(), 
                        //         NumVertices*Stride);
                    }
                    if (void *Data = Section->VertexBuffers.Colors.GetVertexData())
                    {
                        int NumVertices = Section->VertexBuffers.Colors.GetNumVertices();
                        int Stride = Section->VertexBuffers.Colors.GetStride();
                        section["VertexBuffers"]["Colors"]["NumVertices"] = NumVertices;
                        section["VertexBuffers"]["Colors"]["Stride"] = Stride;
                        Buffers.AddBuffer(
                            section["VertexBuffers"]["Colors"]["Data"], 
                            NumVertices*Stride, 
                            Section->VertexBuffers.Colors.GetVertexData());
                        // section["VertexBuffers"]["Colors"]["Data"] = SerializeHelper::Base64Encode(
                        //         (const unsigned char *)Section->VertexBuffers.Colors.GetVertexData(), 
                        //         NumVertices*Stride);
                    }
                    if (void *Data = Section->VertexBuffers.Normals.GetVertexData())
                    {
                        int NumVertices = Section->VertexBuffers.Normals.GetNumVertices();
                        int Stride = Section->VertexBuffers.Normals.GetStride();
                        section["VertexBuffers"]["Normals"]["NumVertices"] = NumVertices;
                        section["VertexBuffers"]["Normals"]["Stride"] = Stride;
                        Buffers.AddBuffer(
                            section["VertexBuffers"]["Normals"]["Data"], 
                            NumVertices*Stride, 
                            Section->VertexBuffers.Normals.GetVertexData());
                        // section["VertexBuffers"]["Normals"]["Data"] = SerializeHelper::Base64Encode(
                        //         (const unsigned char *)Section->VertexBuffers.Normals.GetVertexData(), 
                        //         NumVertices*Stride);
                    }
                    if (void *Data = Section->VertexBuffers.Tangents.GetVertexData())
                    {
                        int NumVertices = Section->VertexBuffers.Tangents.GetNumVertices();
                        int Stride = Section->VertexBuffers.Tangents.GetStride();
                        section["VertexBuffers"]["Tangents"]["NumVertices"] = NumVertices;
                        section["VertexBuffers"]["Tangents"]["Stride"] = Stride;
                        Buffers.AddBuffer(
                            section["VertexBuffers"]["Tangents"]["Data"], 
                            NumVertices*Stride, 
                            Section->VertexBuffers.Tangents.GetVertexData());
                        // section["VertexBuffers"]["Tangents"]["Data"] = SerializeHelper::Base64Encode(
                        //         (const unsigned char *)Data, 
                        //         NumVertices*Stride);
                    }
                    for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
                    {
                        if (void *Data = Section->VertexBuffers.TexCoords[i].GetVertexData())
                        {
                            int NumVertices = Section->VertexBuffers.TexCoords[i].GetNumVertices();
                            int Stride = Section->VertexBuffers.TexCoords[i].GetStride();
                            section["VertexBuffers"]["TexCoords"][i]["NumVertices"] = NumVertices;
                            section["VertexBuffers"]["TexCoords"][i]["Stride"] = Stride;
                            Buffers.AddBuffer(
                                section["VertexBuffers"]["TexCoords"][i]["Data"], 
                                NumVertices*Stride, 
                                Section->VertexBuffers.TexCoords[i].GetVertexData());
                            // section["VertexBuffers"]["TexCoords"][i]["Data"] = SerializeHelper::Base64Encode(
                            //         (const unsigned char *)Data, 
                            //         NumVertices*Stride);
                        }
                    }
                }
            }
        }
        static void Deserialize(FStaticMeshRenderData &Object, nlohmann::json &json, void* Buffer)
        {
            if (!SerializeHelper::CheckIsType(json, "FStaticMeshRenderData")) return;
            
            nlohmann::json &content = json["Content"];

            nlohmann::json &lod_resources = content["LODResources"];

            for (auto &lod_resource : lod_resources)
            {
                auto LODResouce = new FStaticMeshLODResources();
                for (auto &section : lod_resource["Sections"])
                {
                    auto Section = new FStaticMeshSection();

                    if (section.contains("IndexBuffer"))
                    {
                        int NumIndices = section["IndexBuffer"]["NumIndices"];
                        int Stride = section["IndexBuffer"]["Stride"];
                        int BufferOffset = section["IndexBuffer"]["Data"]["BufferOffset"];
                        // std::string Data = SerializeHelper::Base64Decode(section["IndexBuffer"]["Data"]);
                        Section->IndexBuffer.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, NumIndices, Stride);
                    }
                    FStaticVertexFactory::FDataType VFData;
                    if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Positions"))
                    {
                        int NumVertices = section["VertexBuffers"]["Positions"]["NumVertices"];
                        int Stride = section["VertexBuffers"]["Positions"]["Stride"];
                        int BufferOffset = section["VertexBuffers"]["Positions"]["Data"]["BufferOffset"];
                        // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Positions"]["Data"]);
                        Section->VertexBuffers.Positions.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
                        Section->VertexBuffers.Positions.BindToVertexFactoryData(VFData.PositionComponent);
                    }
                    if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Normals"))
                    {
                        int NumVertices = section["VertexBuffers"]["Normals"]["NumVertices"];
                        int Stride = section["VertexBuffers"]["Normals"]["Stride"];
                        int BufferOffset = section["VertexBuffers"]["Normals"]["Data"]["BufferOffset"];
                        // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Normals"]["Data"]);
                        Section->VertexBuffers.Normals.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
                        Section->VertexBuffers.Normals.BindToVertexFactoryData(VFData.NormalComponent);
                    }
                    if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Tangents"))
                    {
                        int NumVertices = section["VertexBuffers"]["Tangents"]["NumVertices"];
                        int Stride = section["VertexBuffers"]["Tangents"]["Stride"];
                        int BufferOffset = section["VertexBuffers"]["Tangents"]["Data"]["BufferOffset"];
                        // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Tangents"]["Data"]);
                        Section->VertexBuffers.Tangents.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
                        Section->VertexBuffers.Tangents.BindToVertexFactoryData(VFData.TangentComponent);
                    }
                    if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("Colors"))
                    {
                        int NumVertices = section["VertexBuffers"]["Colors"]["NumVertices"];
                        int Stride = section["VertexBuffers"]["Colors"]["Stride"];
                        int BufferOffset = section["VertexBuffers"]["Colors"]["Data"]["BufferOffset"];
                        // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["Colors"]["Data"]);
                        Section->VertexBuffers.Colors.Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
                        Section->VertexBuffers.Colors.BindToVertexFactoryData(VFData.ColorComponent);
                    }
                    if (section.contains("VertexBuffers") && section["VertexBuffers"].contains("TexCoords"))
                    {
                        for (int i = 0; i < section["VertexBuffers"]["TexCoords"].size(); i++)
                        {
                            int NumVertices = section["VertexBuffers"]["TexCoords"][i]["NumVertices"];
                            int Stride = section["VertexBuffers"]["TexCoords"][i]["Stride"];
                            int BufferOffset = section["VertexBuffers"]["TexCoords"][i]["Data"]["BufferOffset"];
                            // std::string Data = SerializeHelper::Base64Decode(section["VertexBuffers"]["TexCoords"][i]["Data"]);
                            Section->VertexBuffers.TexCoords[i].Init(static_cast<unsigned char*>(Buffer)+BufferOffset, Stride, NumVertices);
                            Section->VertexBuffers.TexCoords[i].BindToVertexFactoryData(VFData.TexCoordComponent[i]);
                        }
                    }
                    Section->VertexFactory.SetData(VFData);
                    LODResouce->Sections.push_back(Section);
                }
                Object.LODResources.push_back(LODResouce);
            }
        }
    };
}