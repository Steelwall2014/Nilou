#pragma once
#include <vector>

#include "ShaderType.h"
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "RenderGraphDescriptorSet.h"
#include "RenderGraphPass.h"

namespace nilou {

class FDynamicRHI;
class RHICommandList;
class IRHITransientResourceAllocator;

class RenderGraph
{
public:

    friend class RDGBuilder;

    RenderGraph(RHICommandList& InRHICmdList);

    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;

    static RDGTextureRef CreateExternalTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    static RDGTextureViewRef CreateExternalTextureView(const RDGTextureViewDesc& TextureViewDesc);

    static RDGBufferRef CreateExternalBuffer(const std::string& Name, const RDGBufferDesc& Desc);
    
    template<class T>
    static TRDGUniformBufferRef<T> CreateExternalUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc;
        Desc.NumElements = 1;
        Desc.BytesPerElement = sizeof(T);
        return CreateExternalBuffer(Name, Desc);
    }

    static RDGDescriptorSetRef CreateExternalDescriptorSet(RHIDescriptorSetLayout* Layout);

    template<class TShaderType>
    static RDGDescriptorSetRef CreateExternalDescriptorSet(int32 PermutationId, uint32 SetIndex)
    {
        return CreateExternalDescriptorSet(TShaderType::GetDescriptorSetLayout(PermutationId, SetIndex));
    }

    RDGTexture* CreateTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    RDGTextureView* CreateTextureView(const RDGTextureViewDesc& TextureViewDesc);

    RDGBuffer* CreateBuffer(const std::string& Name, const RDGBufferDesc& Desc);
    
    template<class T>
    TRDGUniformBuffer<T>* CreateUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc;
        Desc.NumElements = 1;
        Desc.BytesPerElement = sizeof(T);
        return CreateBuffer(Name, Desc);
    }

    RDGDescriptorSet* CreateDescriptorSet(RHIDescriptorSetLayout* Layout);

    template<class TShaderType>
    RDGDescriptorSet* CreateDescriptorSet(int32 PermutationId, uint32 SetIndex)
    {
        return CreateDescriptorSet(TShaderType::GetDescriptorSetLayout(PermutationId, SetIndex));
    }



    // Add a graphics pass to the render graph
    template <typename ExecuteLambdaType>
    FRDGPassHandle AddGraphicsPass(
        const RDGPassDesc& PassDesc,
        const std::vector<RDGDescriptorSet*>& PassParameters,
        ExecuteLambdaType&& Executor)
    {
        return AddPassInternal(PassDesc, PassParameters, ERHIPipeline::Graphics, std::forward<ExecuteLambdaType>(Executor));
    }

    // Add a compute pass to the render graph
    template <typename ExecuteLambdaType>
    FRDGPassHandle AddComputePass(
        const RDGPassDesc& PassDesc,
        const std::vector<RDGDescriptorSet*>& PassParameters,
        ExecuteLambdaType&& Executor)
    {
        return AddPassInternal(PassDesc, PassParameters, ERHIPipeline::AsyncCompute, std::forward<ExecuteLambdaType>(Executor));
    }

    template <typename SourceType, typename DestinationType, typename ExecuteLambdaType>
    FRDGPassHandle AddCopyPass(
        const RDGPassDesc& PassDesc,
        SourceType* Source,
        DestinationType* Destination,
        ExecuteLambdaType&& Executor)
    {
        // Do some static type checking
        CheckCanCopy(Source, Destination);

        FRDGPass* Pass = new TRDGLambdaPass<ExecuteLambdaType>(
            Passes.size(), 
            PassDesc, 
            ERHIPipeline::Copy,
            std::forward<ExecuteLambdaType>(Executor));

        Passes.push_back(Pass);
        SetupCopyPass(Pass, Source, Destination);
        return Pass->Handle;
    }

    template <typename SourceType, typename DestinationType>
    constexpr void CheckCanCopy(SourceType*, DestinationType*)
    {
        static_assert(std::is_same_v<SourceType, RDGBuffer>, "Source type must be RDGBuffer");
        static_assert(std::is_same_v<DestinationType, RDGBuffer> || std::is_same_v<DestinationType, RDGTextureView>, "Destination type must be RDGBuffer or RDGTextureView");
    }

    void Start();

    void CleanUp();

    void Compile();

    void Execute();

private:

    static RDGTextureView* CreateExternalTextureViewInternal(const RDGTextureViewDesc& TextureViewDesc);

    template <typename ExecuteLambdaType>
    FRDGPassHandle AddPassInternal(
        const RDGPassDesc& PassDesc,
        const std::vector<RDGDescriptorSet*>& PassParameters,
        ERHIPipeline Pipeline,
        ExecuteLambdaType&& Executor)
    {
        FRDGPass* Pass = new TRDGLambdaPass<ExecuteLambdaType>(
            Passes.size(), 
            PassDesc, 
            Pipeline,
            std::forward<ExecuteLambdaType>(Executor));

        Passes.push_back(Pass);
        SetupParameterPass(Pass);
        return Pass->Handle;
    }


	/** The epilogue and prologue passes are sentinels that are used to simplify graph logic around barriers
	*  and traversal. The prologue pass is used exclusively for barriers before the graph executes, while the
	*  epilogue pass is used for resource extraction barriers--a property that also makes it the main root of
	*  the graph for culling purposes. The epilogue pass is added to the very end of the pass array for traversal
	*  purposes. The prologue does not need to participate in any graph traversal behavior.
	*/
	FRDGPass* ProloguePass = nullptr;
	FRDGPass* EpiloguePass = nullptr;

	uint32 AsyncComputePassCount = 0;
	uint32 RasterPassCount = 0;

	FORCEINLINE FRDGPass* GetProloguePass() const
	{
		return ProloguePass;
	}

	/** Returns the graph prologue pass handle. */
	FORCEINLINE FRDGPassHandle GetProloguePassHandle() const
	{
		return FRDGPassHandle(0);
	}

	/** Returns the graph epilogue pass handle. */
	FORCEINLINE FRDGPassHandle GetEpiloguePassHandle() const
	{
		Ncheckf(EpiloguePass, "The handle is not valid until the epilogue has been added to the graph during execution.");
		return Passes.size()-1;
	}

    RHICommandList& RHICmdList;

	/** Registry of graph objects. */
    std::vector<FRDGPass*> Passes;
    std::vector<RDGTextureRef> Textures;
    std::vector<RDGBufferRef> Buffers;
    std::vector<RDGDescriptorSetRef> DescriptorSets;

	/** Tracks external resources to their registered render graph counterparts for de-duplication. */
	std::unordered_map<RHITexture*, RDGTexture*> ExternalTextures;
	std::unordered_map<RHIBuffer*,  RDGBuffer*>  ExternalBuffers;

    static std::map<RHIDescriptorSetLayout*, RDGDescriptorSetPool> DescriptorSetPools;

	std::vector<FRDGPass*> CullPassStack;

    void AddPassDependency(FRDGPass* Producer, FRDGPass* Consumer);

    void SetupPassInternal(FRDGPass* Pass);
    void SetupParameterPass(FRDGPass* Pass);
    void SetupCopyPass(FRDGPass* Pass, RDGResource* Source, RDGResource* Destination);
    void SetupCopyPassResource(FRDGPass* Pass, RDGResource* Resource, ERHIAccess Access);
    void SetupPassResources(FRDGPass* Pass);
	void SetupPassDependencies(FRDGPass* Pass);


    struct FCollectResourceOp
    {
        enum class EOp : uint8
        {
            Allocate,
            Deallocate,
        };

        static FCollectResourceOp Allocate(FRDGPassHandle PassHandle, RDGResource* Resource)
        {
            return FCollectResourceOp(PassHandle, Resource, EOp::Allocate);
        }

        static FCollectResourceOp Deallocate(FRDGPassHandle PassHandle, RDGResource* Resource)
        {
            return FCollectResourceOp(PassHandle, Resource, EOp::Deallocate);
        }

        FCollectResourceOp() = default;
        FCollectResourceOp(FRDGPassHandle InPassHandle, RDGResource* InResource, EOp InOp)
            : PassHandle(InPassHandle)
            , Resource(InResource)
            , Op(InOp)
        {
        }

        FRDGPassHandle PassHandle;
        RDGResource* Resource;
        EOp Op;
    };

    struct FCollectResourceContext
    {
        std::vector<FCollectResourceOp> TransientResources;
        std::vector<FCollectResourceOp> PooledTextures;
		std::vector<FCollectResourceOp> PooledBuffers;
    };

    /** Collects new resource allocations for the pass into the provided context. */
    void CollectAllocations(FCollectResourceContext& Context, FRDGPass* Pass);
    void CollectAllocateTexture(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGTexture* Texture);
    void CollectAllocateBuffer(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGBuffer* Buffer);

    /** Collects new resource deallocations for the pass into the provided context. */
    void CollectDeallocations(FCollectResourceContext& Context, FRDGPass* Pass);
    void CollectDeallocateTexture(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGTexture* Texture, uint32 ReferenceCount);
    void CollectDeallocateBuffer(FCollectResourceContext& Context, FRDGPassHandle PassHandle, RDGBuffer* Buffer, uint32 ReferenceCount);

	/** Allocates resources using the provided lifetime op arrays. */
	void AllocateTransientResources(const std::vector<FCollectResourceOp>& Ops);
	void AllocatePooledTextures(const std::vector<FCollectResourceOp>& Ops);
	void AllocatePooledBuffers(const std::vector<FCollectResourceOp>& Ops);

    IRHITransientResourceAllocator* TransientResourceAllocator = nullptr;

    FRDGPooledTextureRef AllocatePooledRenderTargetRHI(RDGTexture* Texture); 
    FRDGPooledBufferRef AllocatePooledBufferRHI(RDGBuffer* Buffer);

    void SetTransientTextureRHI(RDGTexture* Texture, FRHITransientTexture* TransientTexture);
    void SetTransientBufferRHI(RDGBuffer* Buffer, FRHITransientBuffer* TransientBuffer);

    void CompilePassBarriers();
    void CollectPassBarriers();
    void CollectPassBarriers(FRDGPassHandle PassHandle);

    void ExecuteSerialPass(RHICommandList& RHICmdList, FRDGPass* Pass);

};

}