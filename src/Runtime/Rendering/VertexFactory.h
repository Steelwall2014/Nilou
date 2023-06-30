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
#include "ShaderType.h"
#include "ShaderPermutation.h"

namespace nilou {

    class FVertexFactory
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
    
        std::vector<FVertexInputStream> GetVertexInputStreams() const;
        std::vector<FVertexElement> GetVertexElements() const { return Elements; }
        FRHIVertexDeclaration* GetVertexDeclaration() const { return Declaration; }

        const std::string &GetName() const { return Name; }

        virtual int32 GetPermutationId() const { return 0; }

        virtual void InitVertexFactory() { }

        struct FVertexStream
        {
            const FVertexBuffer* VertexBuffer = nullptr;
            uint32 Offset = 0;
            uint16 Stride = 0;

            friend bool operator==(const FVertexStream& A,const FVertexStream& B)
            {
                return A.VertexBuffer == B.VertexBuffer && A.Stride == B.Stride && A.Offset == B.Offset;
            }

            FVertexStream()
            {
            }
        };

        static FVertexElement AccessStreamComponent(const FVertexStreamComponent& Component, uint8 AttributeIndex, std::vector<FVertexStream>& InOutStreams)
        {
            FVertexStream VertexStream;
            VertexStream.VertexBuffer = Component.VertexBuffer;
            VertexStream.Stride = Component.Stride;
            VertexStream.Offset = Component.Offset;
            auto iter = std::find(InOutStreams.begin(), InOutStreams.end(), VertexStream);
            uint8 StreamIndex;
            if (iter != InOutStreams.end())
            {
                StreamIndex = iter - InOutStreams.begin();
            }
            else 
            {
                InOutStreams.push_back(VertexStream);
                StreamIndex = InOutStreams.size()-1;
            }
            return FVertexElement(StreamIndex, Component.Offset, Component.Type, AttributeIndex, VertexStream.Stride);
        }
        
    protected:
        std::string Name;

        std::vector<FVertexStream> Streams;

        std::vector<FVertexElement> Elements;

        FRHIVertexDeclaration* Declaration;
    };


}