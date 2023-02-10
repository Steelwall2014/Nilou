#pragma once

#include <filesystem>
#include <functional>
#include <set>
#include <string>

#include "UniformBuffer.h"
#include "RHIResources.h"
#include "RenderResource.h"
#include "Templates/ObjectMacros.h"
#include "RHI.h"
#include "HashedName.h"
#include "ShaderCompiler.h"
#include "ShaderSegment.h"
#include "ShaderType.h"
#include "ShaderPermutation.h"

namespace nilou {

    struct FVertexInputStream
    {
        //uint32 StreamIndex : 4;
        uint32 Offset/* : 28*/;
        RHIBuffer* VertexBuffer;

        FVertexInputStream() :
            //StreamIndex(0),
            Offset(0),
            VertexBuffer(nullptr)
        {}

        FVertexInputStream(/*uint32 InStreamIndex, */uint32 InOffset, RHIBuffer* InVertexBuffer)
            : /*StreamIndex(InStreamIndex), */Offset(InOffset), VertexBuffer(InVertexBuffer)
        {
        }

        inline bool operator==(const FVertexInputStream& rhs) const
        {
            if (/*StreamIndex != rhs.StreamIndex ||*/
                Offset != rhs.Offset || 
                VertexBuffer != rhs.VertexBuffer) 
            {
                return false;
            }

            return true;
        }

        inline bool operator!=(const FVertexInputStream& rhs) const
        {
            return !(*this == rhs);
        }
    };

    class FVertexStreamComponent
    {
    public:
        FVertexBuffer *VertexBuffer;
        uint8 Offset;
        uint8 Stride;
        EVertexElementType Type;

        FVertexStreamComponent() 
            : VertexBuffer(nullptr)
            , Offset(0)
            , Stride(0)
            , Type(EVertexElementType::VET_None)
        { }

        FVertexStreamComponent(FVertexBuffer *InVertexBuffer, uint8 InOffset, uint8 InStride, EVertexElementType InType) 
            : VertexBuffer(InVertexBuffer)
            , Offset(InOffset)
            , Stride(InStride)
            , Type(InType)
        { }
    };
    using FVertexStreamComponentList = std::vector<FVertexStreamComponent *>;

    class FVertexFactory// : public FShaderSegment
    {
    /*==============FVertexFactoryType Interface============*/
    public: 
        static FVertexFactoryType StaticType; 
        virtual FVertexFactoryType* GetType() const;
    /*==============FVertexFactoryType Interface============*/
    public:

        using FPermutationDomain = FShaderPermutationNone;
        // using FPermutationParameters = FVertexFactoryPermutationParameters;

        FVertexFactory() { }

        static bool ShouldCompilePermutation(const FVertexFactoryPermutationParameters &Parameters) { return true; }

        static void ModifyCompilationEnvironment(const FVertexFactoryPermutationParameters &Parameters, FShaderCompilerEnvironment &OutEnvironment) { }
    
        /** Override this to implement child VertexFactory */
        virtual void GetVertexInputList(std::vector<FRHIVertexInput> &OutVertexInputs) { }

        const std::string &GetName() { return Name; }

        virtual int32 GetPermutationId() { return 0; }
        
    protected:
        std::string Name;
    };


}