#include "StaticMeshResources.h"
#include "Common/DynamicMeshResources.h"
#include "DynamicRHI.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include <set>

namespace nilou {

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

    }

    bool FStaticVertexFactory::ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters)
    {
        return true;
    }

    void FStaticVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) 
    { 
    }
    
    // void FStaticVertexFactory::SetParameter(const std::string &ParamName, TUniformBuffer<FPrimitiveShaderParameters> *PrimitiveUniformBuffer)
    // {
    //     UniformBuffers[ParamName] = PrimitiveUniformBuffer;
    // }

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
        if (VertexBuffers.Colors.GetVertexData() != nullptr)
            BeginInitResource(&VertexBuffers.Colors);
        if (VertexBuffers.Positions.GetVertexData() != nullptr)
            BeginInitResource(&VertexBuffers.Positions);
        if (VertexBuffers.Normals.GetVertexData() != nullptr)
            BeginInitResource(&VertexBuffers.Normals);
        if (VertexBuffers.Tangents.GetVertexData() != nullptr)
            BeginInitResource(&VertexBuffers.Tangents);
        if (VertexBuffers.Colors.GetVertexData() != nullptr)
            BeginInitResource(&VertexBuffers.Colors);
        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            if (VertexBuffers.TexCoords[i].GetVertexData() != nullptr)
                BeginInitResource(&VertexBuffers.TexCoords[i]);
        }
        if (IndexBuffer.GetIndiceData() != nullptr)
            BeginInitResource(&IndexBuffer);
        bIsInitialized = true;
    }

    void FStaticMeshLODResources::ReleaseResources()
    {
        bIsInitialized = false;
        if (VertexBuffers.Colors.GetVertexData() != nullptr)
            BeginReleaseResource(&VertexBuffers.Colors);
        if (VertexBuffers.Positions.GetVertexData() != nullptr)
            BeginReleaseResource(&VertexBuffers.Positions);
        if (VertexBuffers.Normals.GetVertexData() != nullptr)
            BeginReleaseResource(&VertexBuffers.Normals);
        if (VertexBuffers.Tangents.GetVertexData() != nullptr)
            BeginReleaseResource(&VertexBuffers.Tangents);
        if (VertexBuffers.Colors.GetVertexData() != nullptr)
            BeginReleaseResource(&VertexBuffers.Colors);
        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            if (VertexBuffers.TexCoords[i].GetVertexData() != nullptr)
                BeginReleaseResource(&VertexBuffers.TexCoords[i]);
        }
        if (IndexBuffer.GetIndiceData() != nullptr)
            BeginReleaseResource(&IndexBuffer);
    }

    int32 FStaticMeshLODResources::GetNumVertices() const
    {
        return VertexBuffers.Positions.GetNumVertices();
    }

    // int32 FStaticMeshLODResources::GetNumTexCoords() const
    // {
    //     return VertexBuffers.TexCoords[0].GetNumVertices();
    // }
    
}
