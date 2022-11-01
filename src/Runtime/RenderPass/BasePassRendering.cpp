#include <algorithm>
#include "Material.h"
#include "BasePassRendering.h"
#include "Common/AssertionMacros.h"
#include "DynamicRHI.h"
#include "RHIResources.h"
#include "Shader.h"
#include "ShaderMap.h"
// #include "Shadinclude.h"
#include "Templates/ObjectMacros.h"
#include "VertexFactory.h"

namespace nilou {
    //FBasePassVS *FBasePassVS::Shader = new FBasePassVS;
    
    // const RHIVertexShader *FBasePassVS::GetOrCreateShaderByPermutation(const FVertexFactory *VertexFactory, const std::set<std::string> &Permutation)
    // {
    //     FVertexFactoryType *VertexFactoryType = VertexFactory->GetType();
    //     if (CachedDefinitions.find(VertexFactoryType) == CachedDefinitions.end())
    //     {
    //         std::set<std::string> merged_definitions;
    //         std::set_union(VertexFactory->PossibleDefinitions.begin(), VertexFactory->PossibleDefinitions.end(), 
    //                         this->PossibleDefinitions.begin(), this->PossibleDefinitions.end(), merged_definitions.begin());
    //         CachedDefinitions[VertexFactoryType] = merged_definitions;
    //     }
    //     if (!IsPermutationValid(CachedDefinitions[VertexFactoryType], Permutation))
    //         return nullptr;

    //     std::map<std::set<std::string>, RHIVertexShaderRef> &CurrentVertexFactoryShaderMap = CachedShaderMap[VertexFactoryType];
    //     if (CurrentVertexFactoryShaderMap.find(Permutation) == CurrentVertexFactoryShaderMap.end())
    //     {
    //         std::string FullCode = VertexFactoryType->ShaderSourceCodeBody + GetType()->ShaderSourceCodeBody;
    //         FullCode = Shadinclude::Preprocess(FullCode, "");
    //         RHIVertexShaderRef ShaderRHI = GDynamicRHI->RHICreateVertexShader(FullCode.c_str());
    //         CurrentVertexFactoryShaderMap[Permutation] = ShaderRHI;
    //     }
    //     return CurrentVertexFactoryShaderMap[Permutation].get();
    // }

    // IMPLEMENT_SHADER_TYPE(FBasePassVS, "BasePassVertexShader.vert")
    IMPLEMENT_SHADER_TYPE(FBasePassVS, "/Shaders/MaterialShaders/BasePassVertexShader.vert", EShaderFrequency::SF_Vertex, Material);
    
    IMPLEMENT_SHADER_TYPE(FBasePassPS, "/Shaders/MaterialShaders/BasePassPixelShader.frag", EShaderFrequency::SF_Pixel, Material);

    // void FBasePassMeshPassProcessor::BuildMeshDrawCommands(
    //         const FSceneView &View, 
    //         const std::vector<FMeshBatch> &MeshBatches, 
    //         std::vector<FMeshDrawCommand> &OutMeshDrawCommands)
    // {
    //     for (auto &&Mesh : MeshBatches)
    //     {
    //         FRHIGraphicsPipelineInitializer StateData;

    //         FBasePassVS::FPermutationDomain PermutationVectorVS;
    //         FShaderPermutationParameters PermutationParameters;
    //         PermutationParameters.ShaderType = &FBasePassVS::StaticType;
    //         PermutationParameters.PermutationId = PermutationVectorVS.ToDimensionValueId();
    //         FShaderInstance *VertexShader = GetVertexShaderInstance(*Mesh.MaterialRenderProxy->GetType(), *Mesh.VertexFactory->GetType(), PermutationParameters);

    //         FBasePassPS::FPermutationDomain PermutationVectorPS;
    //         PermutationParameters.ShaderType = &FBasePassPS::StaticType;
    //         PermutationParameters.PermutationId = PermutationVectorPS.ToDimensionValueId();
    //         FShaderInstance *PixelShader = GetPixelShaderInstance(*Mesh.MaterialRenderProxy->GetType(), PermutationParameters);

    //         RHIDepthStencilStateRef DepthState = GDynamicRHI->RHICreateDepthStencilState(Mesh.MaterialRenderProxy->DepthStencilState);
    //         StateData.DepthStentilState = DepthState;

    //         RHIRasterizerStateRef RasterizerState = GDynamicRHI->RHICreateRasterizerState(Mesh.MaterialRenderProxy->RasterizerState);
    //         StateData.RasterizerState = RasterizerState;

    //         std::map<FShaderParameterInfo, RHIUniformBuffer *> UniformBufferBindings; 
    //         std::map<FShaderParameterInfo, FRHISampler *> SamplerBindings; 
    //         std::vector<FRHIVertexInput *> VertexInputs; 
    //         Mesh.MaterialRenderProxy->CollectShaderBindings(UniformBufferBindings, SamplerBindings);
    //         Mesh.VertexFactory->CollectShaderBindings(UniformBufferBindings, SamplerBindings, VertexInputs);

    //         for (auto &MeshElement : Mesh.Elements)
    //         {
    //             FMeshDrawCommand MeshDrawCommand;
    //             MeshDrawCommand.PipelineState = GDynamicRHI->RHIGetOrCreatePipelineStateObject(StateData);
    //             MeshDrawCommand.IndexBuffer = MeshElement.IndexBuffer->IndexBufferRHI.get();
    //             MeshDrawCommand.ShaderBindings.UniformBufferBindings = UniformBufferBindings;
    //             MeshDrawCommand.ShaderBindings.SamplerBindings = SamplerBindings;
    //             MeshDrawCommand.ShaderBindings.VertexAttributeBindings = VertexInputs;
    //             if (MeshElement.NumVertices == 0)
    //             {
    //                 MeshDrawCommand.IndirectArgs.Buffer = MeshElement.IndirectArgsBuffer;
    //                 MeshDrawCommand.IndirectArgs.Offset = MeshElement.IndirectArgsOffset;
    //                 MeshDrawCommand.UseIndirect = true;
    //             }
    //             else
    //             {
    //                 MeshDrawCommand.DirectArgs.NumInstances = MeshElement.NumInstances;
    //                 MeshDrawCommand.DirectArgs.NumVertices = MeshElement.NumVertices;
    //                 MeshDrawCommand.UseIndirect = false;
    //             }
    //         }
    //     }

    // }
}