#pragma once

#include <filesystem>
#include <set>
#include <string>
#include <functional>

#include "HashedName.h"
#include "ShaderParameter.h"
#include "ShaderCompileEnvironment.h"
#include "ShaderParser.h"

namespace nilou {

    std::vector<class FShaderType *> &GetAllShaderTypes();

    std::vector<class FVertexFactoryType *> &GetAllVertexFactoryTypes();

    enum class EShaderFrequency
    {
        SF_None,
        SF_Vertex,
        SF_Pixel,
        SF_Compute
    };



    enum class EShaderMetaType
    {
        SMT_None,
        SMT_Global,
        SMT_Material
    };

    class FShaderParameterCode
    {
    public:
        std::string Name;
        EShaderParameterType ParameterType;
        std::string Code;
        bool operator<(const FShaderParameterCode &Other) const
        {
            return Name < Other.Name;
        }
        bool operator==(const FShaderParameterCode &Other) const
        {
            return Name == Other.Name;
        }
    };

    // struct FShaderCodeInitializer
    // {
    //     std::set<FShaderParameterCode> ParameterCodes;
    //     std::string SourceCodeBody;
    // };

    struct FShaderTypeBase
    {
        std::string Name;
        std::string VirtualFilePath;
        // std::string SourceCodeBody;
        std::filesystem::path FileAbsolutePath;
        FHashedName HashedName;
        // std::set<FShaderParameterCode> ShaderParameterCodes;
        // FShaderCodeInitializer CodeInitializer;
        std::string PreprocessedCode;
        int32 PermutationCount;
        
        FShaderTypeBase::FShaderTypeBase() { }

        /** 
        Some preprocess operations will be done in the constructor, 
        like change the relative include path to absolute path and discard the #version line
        */
        FShaderTypeBase::FShaderTypeBase(const std::string &InClassName, const std::string &InFileName, int32 InPermutationCount);

        void ReadSourceCode();

        FHashedName GetHashedFileName() const
        {
            return HashedName;
        }
    private:
        bool bSourceCodeReaded = false;
    };

    struct FShaderPermutationParameters
    {
        class FShaderType *Type;
        int32 PermutationId;
        FShaderPermutationParameters(FShaderType *InType=nullptr, int32 InPermutationId=0)
            : Type(InType)
            , PermutationId(InPermutationId)
        {}
    };

    struct FShaderType : public FShaderTypeBase
    {
    public:
        friend class FShaderCompiler;
        EShaderFrequency ShaderFrequency;
        EShaderMetaType ShaderMetaType;

        FShaderType() {}
        
        FShaderType::FShaderType(
            const std::string &InShaderClassName, 
            const std::string &InShaderFileName, 
            EShaderFrequency InShaderFrequency, 
            EShaderMetaType InShaderMetaType,
            std::function<bool(const FShaderPermutationParameters&)> InShouldCompilePermutation,
            std::function<void(const const FShaderPermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
            int32 InPermutationCount
        )
            : FShaderTypeBase(InShaderClassName, InShaderFileName, InPermutationCount)
            , ShaderFrequency(InShaderFrequency)
            , ShaderMetaType(InShaderMetaType)
            , ShouldCompilePermutation(InShouldCompilePermutation)
            , ModifyCompilationEnvironment(InModifyCompilationEnvironment)
        { 
            GetAllShaderTypes().push_back(this);
        }

        std::function<bool(const FShaderPermutationParameters&)> ShouldCompilePermutation;
        std::function<void(const const FShaderPermutationParameters&, FShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
        //bool (*ShouldCompilePermutation)(const FShaderPermutationParameters&);
        //void (*ModifyCompilationEnvironment)(const FShaderPermutationParameters&, FShaderCompilerEnvironment&);
    };

    

    struct FVertexFactoryPermutationParameters
    {
        class FVertexFactoryType *Type;
        int32 PermutationId;
        FVertexFactoryPermutationParameters(FVertexFactoryType *InType=nullptr, int32 InPermutationId=0)
            : Type(InType)
            , PermutationId(InPermutationId)
        {}
    };

    struct FVertexFactoryType : public FShaderTypeBase
    {
    public:
        friend class FShaderCompiler;

        FVertexFactoryType() {}

        FVertexFactoryType::FVertexFactoryType(
            const std::string &InFactoryName, 
            const std::string &InShaderFileName,
            std::function<bool(const FVertexFactoryPermutationParameters&)> InShouldCompilePermutation,
            std::function<void(const const FVertexFactoryPermutationParameters&, FShaderCompilerEnvironment&)> InModifyCompilationEnvironment,
            int32 InPermutationCount)
            : FShaderTypeBase(InFactoryName, InShaderFileName, InPermutationCount)
            , ShouldCompilePermutation(InShouldCompilePermutation)
            , ModifyCompilationEnvironment(InModifyCompilationEnvironment)
        {
            GetAllVertexFactoryTypes().push_back(this);
        }
        std::function<bool(const FVertexFactoryPermutationParameters&)> ShouldCompilePermutation;
        std::function<void(const const FVertexFactoryPermutationParameters&, FShaderCompilerEnvironment&)> ModifyCompilationEnvironment;
    };
}