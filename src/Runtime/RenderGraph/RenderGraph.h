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

    RenderGraph();

    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;
    RenderGraph& operator=(RenderGraph&&) = default;

    static RDGTextureRef CreateExternalTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    static RDGTextureViewRef CreateExternalTextureView(const std::string& Name, RDGTexture* Texture, const RDGTextureViewDesc& TextureViewDesc);

    static RDGTextureViewRef CreateExternalTextureView(RDGTexture* Texture);

    static RDGBufferRef CreateExternalBuffer(const std::string& Name, const RDGBufferDesc& Desc);
    
    template<class T>
    static TRDGUniformBufferRef<T> CreateExternalUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc;
        Desc.NumElements = 1;
        Desc.BytesPerElement = sizeof(T);
        TRDGUniformBufferRef<T> Buffer = new TRDGUniformBuffer<T>(Name, Desc);
        Buffer->bIsPersistent = true;
        Buffer->ResourceRHI = RHICreateBuffer(Desc.GetStride(), Desc.GetStride(), EBufferUsageFlags::None, nullptr);
        return Buffer;
    }

    static RDGDescriptorSetRef CreateExternalDescriptorSet(RHIDescriptorSetLayout* Layout);

    template<class TShaderType>
    static RDGDescriptorSetRef CreateExternalDescriptorSet(int32 PermutationId, uint32 SetIndex)
    {
        return CreateExternalDescriptorSet(TShaderType::GetDescriptorSetLayout(PermutationId, SetIndex));
    }

    RDGTexture* CreateTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    RDGTextureView* CreateTextureView(const std::string& Name, RDGTexture* Texture, const RDGTextureViewDesc& TextureViewDesc);

    RDGTextureView* CreateTextureView(RDGTexture* Texture);

    RDGBuffer* CreateBuffer(const std::string& Name, const RDGBufferDesc& Desc);
    
    template<class T>
    TRDGUniformBuffer<T>* CreateUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc;
        Desc.NumElements = 1;
        Desc.BytesPerElement = sizeof(T);
        TRDGUniformBufferRef<T> Buffer = new TRDGUniformBuffer<T>(Name, Desc);
        Buffer->bDirty = true;
        Buffers.push_back(Buffer);
        return Buffer;
    }

    template<class TShaderType>
    RDGDescriptorSet* CreateDescriptorSet(int32 PermutationId, uint32 SetIndex)
    {
        RHIDescriptorSetLayout* Layout = TShaderType::GetDescriptorSetLayout(PermutationId, SetIndex);
        RDGDescriptorSet* DescriptorSet = CreateDescriptorSet(Layout);
        DescriptorSet->SetIndex = SetIndex;
        return DescriptorSet;
    }



    // Add a graphics pass to the render graph
    template <typename ExecuteLambdaType>
    FRDGPassHandle AddGraphicsPass(
        const RDGPassDesc& PassDesc,
        const RDGRenderTargets& RenderTargets,
        const std::set<RDGDescriptorSet*>& PassParameters,
        ExecuteLambdaType&& Executor)
    {
        return AddPassInternal(PassDesc, PassParameters, ERHIPipeline::Graphics, std::forward<ExecuteLambdaType>(Executor));
    }

    // Add a compute pass to the render graph
    template <typename ExecuteLambdaType>
    FRDGPassHandle AddComputePass(
        const RDGPassDesc& PassDesc,
        const std::set<RDGDescriptorSet*>& PassParameters,
        ExecuteLambdaType&& Executor)
    {
        return AddPassInternal(PassDesc, PassParameters, ERHIPipeline::AsyncCompute, std::forward<ExecuteLambdaType>(Executor));
    }

    template <typename ExecuteLambdaType>
    FRDGPassHandle AddCopyPass(
        const RDGPassDesc& PassDesc,
        RDGBuffer* Source,
        RDGTexture* Destination,
        ExecuteLambdaType&& Executor)
    {
        FRDGPass* Pass = new TRDGLambdaPass<ExecuteLambdaType>(
            Passes.size(), 
            PassDesc, 
            ERHIPipeline::Copy,
            std::forward<ExecuteLambdaType>(Executor));

        Passes.push_back(Pass);
        SetupCopyPass(Pass, Source, Destination);
        return Pass->Handle;
    }

    void Start();

    void CleanUp();

    void Compile();

    void Execute();

private:

    RDGDescriptorSet* CreateDescriptorSet(RHIDescriptorSetLayout* Layout);

    template <typename ExecuteLambdaType>
    FRDGPassHandle AddPassInternal(
        const RDGPassDesc& PassDesc,
        const std::set<RDGDescriptorSet*>& PassParameters,
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

    // RHICommandList& RHICmdList;

	/** Registry of graph objects. */
    std::vector<FRDGPass*> Passes;
    std::vector<RDGTextureRef> Textures;
    std::vector<RDGTextureViewRef> TextureViews;
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
		std::unordered_set<RDGTextureView*> Views;
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
    void CreateViews(const std::unordered_set<RDGTextureView*>& ViewsToCreate);

    void SetTransientTextureRHI(RDGTexture* Texture, FRHITransientTexture* TransientTexture);
    void SetTransientBufferRHI(RDGBuffer* Buffer, FRHITransientBuffer* TransientBuffer);

    void CompilePassBarriers();
    void CollectPassBarriers();
    void CollectPassBarriers(FRDGPassHandle PassHandle);

    void ExecuteSerialPass(RHICommandList& RHICmdList, FRDGPass* Pass);

    static void EnumerateTextureAccess(const std::vector<RDGDescriptorSet*>& PassParameters, const std::function<void(RDGTextureView*,RDGTexture*,RHISamplerState*,ERHIAccess)>& AccessFunction);
    static void EnumerateBufferAccess(const std::vector<RDGDescriptorSet*>& PassParameters, const std::function<void(RDGBuffer*,ERHIAccess)>& AccessFunction);


};

}