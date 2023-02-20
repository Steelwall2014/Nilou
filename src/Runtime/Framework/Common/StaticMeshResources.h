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

        virtual void GetVertexInputList(std::vector<FRHIVertexInput> &OutVertexInputs) const override;

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
    };

    struct FStaticMeshSection
    {
        /** The index of the material with which to render this section. */
        int32 MaterialIndex;

        FStaticMeshVertexBuffers VertexBuffers;

        FStaticMeshIndexBuffer IndexBuffer;

        FStaticVertexFactory VertexFactory;

        /** If true, this section will cast a shadow. */
        bool bCastShadow;

        int32 GetNumVertices() const;
    };

    struct FStaticMeshLODResources
    {
    public:

        std::vector<std::unique_ptr<FStaticMeshSection>> Sections;

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
        std::vector<std::shared_ptr<FStaticMeshLODResources>> LODResources;

        ~FStaticMeshRenderData()
        {
            for (std::shared_ptr<FStaticMeshLODResources> Resource : LODResources)
            {
                Resource->ReleaseResources();
            }
        }

        bool IsInitialized() 
        { 
            for (std::shared_ptr<FStaticMeshLODResources> Resource : LODResources)
            {
                if (!Resource->IsInitialized())
                    return false;
            }
            return true;
        }
        
        void InitResources()
        {
            for (std::shared_ptr<FStaticMeshLODResources> Resource : LODResources)
            {
                Resource->InitResources();
            }
        }
    };

    class UStaticMesh
    {
    public:
        UStaticMesh(const std::string &Name) : MeshName(Name) { }
        std::string MeshName;
        std::unique_ptr<FStaticMeshRenderData> RenderData;
        std::vector<FMaterial *> MaterialSlots;
        FBoundingBox LocalBoundingBox;
        ~UStaticMesh() { ReleaseRenderResources(); }
        void ReleaseRenderResources();
    };
}