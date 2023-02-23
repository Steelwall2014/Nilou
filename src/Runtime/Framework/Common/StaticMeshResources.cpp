#include "StaticMeshResources.h"
#include "Common/DynamicMeshResources.h"
#include "Common/ContentManager.h"
#include "Material.h"
#include "DynamicRHI.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include <set>

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

    void FStaticVertexFactory::GetVertexInputList(std::vector<FRHIVertexInput> &OutVertexInputs) const
    {
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
    }

    bool FStaticVertexFactory::ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters)
    {
        return true;
    }

    void FStaticVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) 
    { 
    }

    IMPLEMENT_VERTEX_FACTORY_TYPE(FStaticVertexFactory, "/Shaders/VertexFactories/StaticMeshVertexFactory.glsl")


    // void FPositionOnlyVertexFactory::GetVertexInputList()
    // {
	//     // FVertexDeclarationElementList Elements;
    //     VertexInputList.clear();
    //     if (Data.PositionComponent.VertexBuffer != nullptr)
    //     {
    //         VertexInputList.push_back(AccessStreamComponent(Data.PositionComponent, 0));
    //     }
    //     // return Elements;
    // }

    // void FPositionOnlyVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) 
    // { 

    // }
    // IMPLEMENT_VERTEX_FACTORY_TYPE(FPositionOnlyVertexFactory, "/Shaders/VertexFactories/StaticMeshVertexFactory.glsl")
    

    // void FPositionAndNormalOnlyVertexFactory::GetVertexInputList()
    // {
	//     // FVertexDeclarationElementList Elements;
    //     VertexInputList.clear();
    //     if (Data.PositionComponent.VertexBuffer != nullptr)
    //     {
    //         VertexInputList.push_back(AccessStreamComponent(Data.PositionComponent, 0));
    //     }
    //     if (Data.NormalComponent.VertexBuffer != nullptr)
    //     {
    //         VertexInputList.push_back(AccessStreamComponent(Data.NormalComponent, 1));
    //     }
    //     // return Elements;
    // }

    // void FPositionAndNormalOnlyVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) 
    // { 
    //     OutEnvironment.SetDefine("SUPPORTS_NORMAL_VERT");
    // }
    // IMPLEMENT_VERTEX_FACTORY_TYPE(FPositionAndNormalOnlyVertexFactory, "/Shaders/VertexFactories/StaticMeshVertexFactory.glsl")

    void FStaticMeshLODResources::InitResources()
    {
        for (int SectionIndex = 0; SectionIndex < Sections.size(); SectionIndex++)
        {
            FStaticMeshSection &Section = *Sections[SectionIndex].get();
            if (Section.VertexBuffers.Colors.GetVertexData() != nullptr)
                BeginInitResource(&Section.VertexBuffers.Colors);
            if (Section.VertexBuffers.Positions.GetVertexData() != nullptr)
                BeginInitResource(&Section.VertexBuffers.Positions);
            if (Section.VertexBuffers.Normals.GetVertexData() != nullptr)
                BeginInitResource(&Section.VertexBuffers.Normals);
            if (Section.VertexBuffers.Tangents.GetVertexData() != nullptr)
                BeginInitResource(&Section.VertexBuffers.Tangents);
            if (Section.VertexBuffers.Colors.GetVertexData() != nullptr)
                BeginInitResource(&Section.VertexBuffers.Colors);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
            {
                if (Section.VertexBuffers.TexCoords[i].GetVertexData() != nullptr)
                    BeginInitResource(&Section.VertexBuffers.TexCoords[i]);
            }
            if (Section.IndexBuffer.GetIndiceData() != nullptr)
                BeginInitResource(&Section.IndexBuffer);
        }
        bIsInitialized = true;
    }

    void FStaticMeshLODResources::ReleaseResources()
    {
        bIsInitialized = false;
        for (int SectionIndex = 0; SectionIndex < Sections.size(); SectionIndex++)
        {
            FStaticMeshSection &Section = *Sections[SectionIndex].get();
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

    void UStaticMesh::ReleaseRenderResources()
    {
        for (int i = 0; i < RenderData->LODResources.size(); i++)
        {
            RenderData->LODResources[i]->ReleaseResources();
        }
    }

    void UStaticMesh::Serialize(nlohmann::json &json, const std::filesystem::path &Path)
    {
        json["ClassName"] = "UStaticMesh";
        nlohmann::json &content = json["Content"];
        content["Name"] = Name;
        TStaticSerializer<FBoundingBox>::Serialize(LocalBoundingBox, content["LocalBoundingBox"]);
        TStaticSerializer<FStaticMeshRenderData>::Serialize(*RenderData, content["RenderData"]);
    }

    void UStaticMesh::Deserialize(nlohmann::json &json, const std::filesystem::path &InPath)
    {
        if (!SerializeHelper::CheckIsType(json, "UStaticMesh")) return;
        nlohmann::json &content = json["Content"];
        Name = content["Name"];
        Path = InPath;
        TStaticSerializer<FBoundingBox>::Deserialize(LocalBoundingBox, content["LocalBoundingBox"]);
        TStaticSerializer<FStaticMeshRenderData>::Deserialize(*RenderData, content["RenderData"]);
        RenderData->InitResources();
        for (int i = 0; i < content["MaterialSlots"].size(); i++)
        {
            fs::path material_path = content["MaterialSlots"][i].get<std::string>();
            material_path = fs::weakly_canonical(Path.parent_path() / fs::path(material_path));
            UMaterial *Material = dynamic_cast<UMaterial *>(GetContentManager()->GetContentByPath(material_path));
            MaterialSlots.push_back(Material);
        }
    }
    
}
