#pragma once

#include "Shader.h"
#include "Templates/ObjectMacros.h"
#include "VertexFactory.h"
#include "RenderPass.h"

namespace nilou {

    class FBasePassVS : public FMaterialShader
    {
        DECLARE_SHADER_TYPE()
    public:
        
    };

    class FBasePassPS : public FMaterialShader
    {
        DECLARE_SHADER_TYPE()
    public:
        
    };
    // class FBasePassVS : public FShader
    // {
    //     DECLARE_SHADER_TYPE()
    // public:
    //     FBasePassVS()
    //     { 
    //     }

    //     const RHIVertexShader *GetOrCreateShaderByPermutation(const FVertexFactory *VertexFactory, const std::set<std::string> &Permutation);


    //     static FBasePassVS *Shader;

    // protected:

    //     std::map<FVertexFactoryType *, std::map<std::set<std::string>, RHIVertexShaderRef>> CachedShaderMap;
    //     std::map<FVertexFactoryType *, std::set<std::string>> CachedDefinitions;

    //     inline virtual void INITDEFINITIONS() override
    //     {
    //         PossibleDefinitions = {
    //             "SUPPORTS_TEXCOORD",
    //             "SUPPORTS_TANGENT"
    //         };
    //     }
    // };

    // class FBasePassMeshPassProcessor : public FMeshPassProcessor
    // {
    // public: 
    //     virtual void BuildMeshDrawCommands(
    //         const FSceneView &View, 
    //         const std::vector<FMeshBatch> &MeshBatches, 
    //         std::vector<FMeshDrawCommand> &OutMeshDrawCommands);
    // };

}