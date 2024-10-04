#include <regex>
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
#include "PipelineStateCache.h"


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
            BeginInitResource(VertexFactory);
        }
    }

    void FStaticVertexFactory::SetData(const FDataType &InData)
    {
        Data = InData;
    }

    void FStaticVertexFactory::InitRHI(RenderGraph& Graph)
    {
        if (Data.PositionComponent.VertexBuffer != nullptr)
        {
            Elements[0] = AccessStreamComponent(Data.PositionComponent, 0, Streams);
        }
        if (Data.NormalComponent.VertexBuffer != nullptr)
        {
            Elements[1] = AccessStreamComponent(Data.NormalComponent, 1, Streams);
        }
        if (Data.TangentComponent.VertexBuffer != nullptr)
        {
            Elements[2] = AccessStreamComponent(Data.TangentComponent, 2, Streams);
        }
        if (Data.ColorComponent.VertexBuffer != nullptr)
        {
            Elements[3] = AccessStreamComponent(Data.ColorComponent, 3, Streams);
        }
        for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
        {
            if (Data.TexCoordComponent[i].VertexBuffer != nullptr)
            {
                Elements[4+i] = AccessStreamComponent(Data.TexCoordComponent[i], 4+i, Streams);
            }
        }
        Declaration = RHICreateVertexDeclaration(Elements);
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

    int32 FStaticVertexFactory::GetPermutationId() const
    {
        FPermutationDomain Domain;
        Domain.Set<FDimensionEnableColorComponent>(Data.ColorComponent.VertexBuffer != nullptr);
        return Domain.ToDimensionValueId();
    }

    bool FStaticVertexFactory::ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters)
    {
        return true;
    }

    void FStaticVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) 
    { 
        FPermutationDomain Domain(Parameters.PermutationId);
        Domain.ModifyCompilationEnvironment(OutEnvironment);
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
            BeginInitResource(&Section->VertexFactory);
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

    FMeshSectionInfo FMeshSectionInfoMap::Get(int32 LODIndex, int32 SectionIndex) const
    {
        uint32 key = LODIndex << 16 | SectionIndex;
        auto Found = Map.find(key);
        if (Found != Map.end())
        {
            return Found->second;
        }
        return FMeshSectionInfo{SectionIndex};
    }

    void FMeshSectionInfoMap::Set(int32 LODIndex, int32 SectionIndex, FMeshSectionInfo Info)
    {
        uint32 key = LODIndex << 16 | SectionIndex;
        Map[key] = Info;
    }

    int32 FMeshSectionInfoMap::GetSectionNumber(int32 LODIndex) const
    {
	    int32 SectionCount = 0;
        for (auto &Pair : Map)
        {
            if (((Pair.first & 0xffff0000) >> 16) == LODIndex)
            {
                SectionCount++;
            }
        }
        return SectionCount;
    }

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

    void UStaticMesh::Build(const FMeshDescription& MeshDesc)
    {
        // release old resources
        if (RenderData != nullptr)
        {
            delete RenderData;
            RenderData = nullptr;
        }
        SectionInfoMap.Clear();
        LocalBoundingBox = FBoundingBox();
        // TODO: multiple LODs
        NumLODs = 1;
        RenderData = new FStaticMeshRenderData;
        FStaticMeshLODResources* Resource = new FStaticMeshLODResources();
        RenderData->LODResources.push_back(Resource);
        for (int prim_index = 0; prim_index < MeshDesc.Primitives.size(); prim_index++)
        {
            auto& primitive = MeshDesc.Primitives[prim_index];
            FStaticVertexFactory::FDataType Data;
            FStaticMeshSection* Section = new FStaticMeshSection;
            Section->MaterialIndex = primitive.MaterialIndex;

            // Currently the model must have positions, normals, tangents and texcoords
            // TODO: compute normals and tangents if they are not provided.
            Ncheck(primitive.Positions.size() > 0);
            Ncheck(primitive.Normals.size() > 0);
            Ncheck(primitive.Tangents.size() > 0);
            Ncheck(primitive.TexCoords[0].size() > 0);

            // copy index buffer and vertex buffer data
            Section->IndexBuffer.Init(primitive.Indices);
            Section->MaterialIndex = primitive.MaterialIndex;
            Section->VertexBuffers.Positions.Init(primitive.Positions);
            Section->VertexBuffers.Positions.BindToVertexFactoryData(Data.PositionComponent);
            Section->VertexBuffers.Normals.Init(primitive.Normals);
            Section->VertexBuffers.Normals.BindToVertexFactoryData(Data.NormalComponent);
            Section->VertexBuffers.Tangents.Init(primitive.Tangents);
            Section->VertexBuffers.Tangents.BindToVertexFactoryData(Data.TangentComponent);
            Section->VertexBuffers.Colors.Init(primitive.Colors);
            Section->VertexBuffers.Colors.BindToVertexFactoryData(Data.ColorComponent);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
            {
                Section->VertexBuffers.TexCoords[i].Init(primitive.TexCoords[i]);
                Section->VertexBuffers.TexCoords[i].BindToVertexFactoryData(Data.TexCoordComponent[i]);
            }
            Section->VertexFactory.SetData(Data);
            Resource->Sections.push_back(Section);

            // compute bounding box
            for (int vid = 0; vid < primitive.Positions.size(); vid++)
            {
                LocalBoundingBox.Min = glm::min(LocalBoundingBox.Min, dvec3(primitive.Positions[vid]));
                LocalBoundingBox.Max = glm::max(LocalBoundingBox.Max, dvec3(primitive.Positions[vid]));
            }

            // set section info
            FMeshSectionInfo Info;
            Info.MaterialIndex = primitive.MaterialIndex;
            Info.bCastShadow = true;
            // copy section data for serialization
            FStaticMeshSectionData& SectionData = Info.SectionData;
            SectionData.IndexBuffer = FVertexIndexBufferData(Section->IndexBuffer);
            SectionData.Positions = FVertexIndexBufferData(Section->VertexBuffers.Positions);
            SectionData.Normals = FVertexIndexBufferData(Section->VertexBuffers.Normals);
            SectionData.Tangents = FVertexIndexBufferData(Section->VertexBuffers.Tangents);
            SectionData.Colors = FVertexIndexBufferData(Section->VertexBuffers.Colors);
            for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
            {
                SectionData.TexCoords.push_back(FVertexIndexBufferData(Section->VertexBuffers.TexCoords[i]));
            }
            SectionInfoMap.Set(0, prim_index, Info);
        }
        RenderData->InitResources();
    }

    void UStaticMesh::PostSerialize(FArchive& Ar)
    {

    }

    void UStaticMesh::PostDeserialize(FArchive& Ar)
    {
        RenderData = new FStaticMeshRenderData;
        for (int LODIndex = 0; LODIndex < NumLODs; LODIndex++)
        {
            FStaticMeshLODResources* Resource = new FStaticMeshLODResources();
            int NumSections = SectionInfoMap.GetSectionNumber(LODIndex);
            for (int SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
            {
                FMeshSectionInfo Info = SectionInfoMap.Get(LODIndex, SectionIndex);
                FStaticMeshSectionData& SectionData = Info.SectionData;
                FStaticMeshSection* Section = new FStaticMeshSection;
                Section->MaterialIndex = Info.MaterialIndex;
                Section->IndexBuffer.Init(SectionData.IndexBuffer.Data.Buffer.get(), SectionData.IndexBuffer.Stride, SectionData.IndexBuffer.Data.BufferSize / SectionData.IndexBuffer.Stride);
                FStaticVertexFactory::FDataType Data;
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
                load_data(SectionData.Positions, Data.PositionComponent, Section->VertexBuffers.Positions);
                load_data(SectionData.Normals, Data.NormalComponent, Section->VertexBuffers.Normals);
                load_data(SectionData.Colors, Data.ColorComponent, Section->VertexBuffers.Colors);
                load_data(SectionData.Tangents, Data.TangentComponent, Section->VertexBuffers.Tangents);
                for (int i = 0; i < SectionData.TexCoords.size(); i++)
                {
                    load_data(SectionData.TexCoords[i], Data.TexCoordComponent[i], Section->VertexBuffers.TexCoords[i]);
                }
                Section->VertexFactory.SetData(Data);
                Resource->Sections.push_back(Section);
            }
            RenderData->LODResources.push_back(Resource);
        }
        RenderData->InitResources();  
    }
    
}
