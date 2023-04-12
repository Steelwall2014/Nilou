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

    std::vector<FRHIVertexInput> FStaticVertexFactory::GetVertexInputList() const
    {
        std::vector<FRHIVertexInput> OutVertexInputs;
        if (Data.PositionComponent.VertexBuffer != nullptr)
        {
            OutVertexInputs.push_back(AccessStreamComponent(Data.PositionComponent, 0));
        }
        if (Data.NormalComponent.VertexBuffer != nullptr)
        {
            OutVertexInputs.push_back(AccessStreamComponent(Data.NormalComponent, 1));
        }
        if (Data.TangentComponent.VertexBuffer != nullptr)
        {
            OutVertexInputs.push_back(AccessStreamComponent(Data.TangentComponent, 2));
        }
        if (Data.ColorComponent.VertexBuffer != nullptr)
        {
            OutVertexInputs.push_back(AccessStreamComponent(Data.ColorComponent, 3));
        }
        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            if (Data.TexCoordComponent[i].VertexBuffer != nullptr)
            {
                OutVertexInputs.push_back(AccessStreamComponent(Data.TexCoordComponent[i], 4+i));
            }
        }
        return OutVertexInputs;
    }

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
    }

    void UStaticMesh::Serialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        json["ClassName"] = "UStaticMesh";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        TStaticSerializer<FBoundingBox>::Serialize(LocalBoundingBox, content["LocalBoundingBox"], Ar.OutBuffers);
        TStaticSerializer<FStaticMeshRenderData>::Serialize(*RenderData, content["RenderData"], Ar.OutBuffers);
        for (int i = 0; i < MaterialSlots.size(); i++)
        {
            if (!MaterialSlots[i]->SerializationPath.empty())
            {
                content["MaterialSlots"][i] = MaterialSlots[i]->SerializationPath.generic_string();
            }
        }
    }

    void UStaticMesh::Deserialize(FArchive &Ar)
    {
        nlohmann::json &json = Ar.json;
        if (!SerializeHelper::CheckIsType(json, "UStaticMesh")) return;
        nlohmann::json &content = json["Content"];
        Name = content["Name"];
        TStaticSerializer<FBoundingBox>::Deserialize(LocalBoundingBox, content["LocalBoundingBox"], Ar.InBuffer.get());
        TStaticSerializer<FStaticMeshRenderData>::Deserialize(*RenderData, content["RenderData"], Ar.InBuffer.get());
        RenderData->InitResources();
        for (int i = 0; i < content["MaterialSlots"].size(); i++)
        {
            fs::path material_path = content["MaterialSlots"][i].get<std::string>();
            UMaterial *Material = GetContentManager()->GetMaterialByPath(material_path);
            MaterialSlots.push_back(Material);
        }
    }
    
}
