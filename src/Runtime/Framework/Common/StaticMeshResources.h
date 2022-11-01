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

    // class FPositionOnlyVertexFactory : public FVertexFactory
    // {
    //     DECLARE_VERTEX_FACTORY_TYPE(FPositionOnlyVertexFactory)
    // public:

    //     struct FDataType
    //     {
    //         FVertexStreamComponent PositionComponent;
    //     };

    //     virtual void InitRHI() override;
    //     virtual void ReleaseRHI() override
    //     {
	// 	    FRenderResource::ReleaseRHI();
    //     }

	//     void SetData(const FDataType& InData)
    //     {
    //         Data = InData;
    //         UpdateRHI();
    //     }

    //     virtual FVertexDeclarationElementList GetDeclarationElementList() const override;

    //     static void ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment);

    // protected:
    //     FDataType Data;
    // };


    class FStaticVertexFactory : public FVertexFactory
    {
        DECLARE_VERTEX_FACTORY_TYPE(FStaticVertexFactory)
    public:

        FStaticVertexFactory(const std::string_view &InName) : FVertexFactory(InName) { }

        struct FDataType
        {
            FVertexStreamComponent PositionComponent;
            
            FVertexStreamComponent NormalComponent;
            
            FVertexStreamComponent TangentComponent;
            
            FVertexStreamComponent TexCoordComponent[MAX_STATIC_TEXCOORDS];

            FVertexStreamComponent ColorComponent;
        };

	    void SetData(const FDataType& InData);
        // {
        //     Data = InData;
        //     GetVertexInputList();
        // }
        void GetVertexInputList();

        static bool ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters);

        static void ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment);
        
        // void SetParameter(const std::string &ParamName, TUniformBuffer<FPrimitiveShaderParameters> *PrimitiveUniformBuffer);

    protected:
        FDataType Data;

    };

    // class FPositionOnlyVertexFactory : public FStaticVertexFactory
    // {
    //     DECLARE_VERTEX_FACTORY_TYPE(FPositionOnlyVertexFactory)
    // public:
    //     FPositionOnlyVertexFactory(const std::string_view &InName) : FStaticVertexFactory(InName) { }
    //     static void ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment);
    // };

    // class FPositionAndNormalOnlyVertexFactory : public FStaticVertexFactory
    // {
    //     DECLARE_VERTEX_FACTORY_TYPE(FPositionAndNormalOnlyVertexFactory)
    // public:
    //     FPositionAndNormalOnlyVertexFactory(const std::string_view &InName) : FStaticVertexFactory(InName) { }
    //     static void ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment);
    // };

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


    struct FStaticMeshLODResources
    {
    public:

        FStaticMeshVertexBuffers VertexBuffers;

        FStaticMeshIndexBuffer IndexBuffer;

        void InitResources();

        void ReleaseResources();

        int32 GetNumVertices() const;

        // int32 GetNumTexCoords() const;

        bool IsInitialized() { return bIsInitialized; }

    private:
        bool bIsInitialized = false;
    };

    class FStaticMeshRenderData
    {
    public:
        std::vector<std::shared_ptr<FStaticMeshLODResources>> LODResources;
        std::shared_ptr<FStaticVertexFactory> VertexFactory; 

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
        FBoundingBox LocalBoundingBox;
        FMaterial *StaticMaterial;
        // class UMaterial *Material;
    };
}