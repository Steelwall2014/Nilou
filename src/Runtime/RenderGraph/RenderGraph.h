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
    static TRDGUniformBufferRef<T> CreateExternalUniformBuffer(const std::string& Name, const T* Data)
    {
        RDGBufferDesc Desc(sizeof(T), sizeof(T), EBufferUsageFlags::UniformBuffer);
        TRDGUniformBufferRef<T> Buffer = new TRDGUniformBuffer<T>(Name, Desc);
        Buffer->bTransient = false;
        return Buffer;
    }

    static RDGDescriptorSetRef CreateExternalDescriptorSet(std::string Name, RHIDescriptorSetLayout* Layout);

    RDGTexture* CreateTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    RDGTextureView* CreateTextureView(const std::string& Name, RDGTexture* Texture, const RDGTextureViewDesc& TextureViewDesc);

    RDGTextureView* CreateTextureView(RDGTexture* Texture);

    RDGBuffer* CreateBuffer(const std::string& Name, const RDGBufferDesc& Desc);

    void QueueBufferUpload(RDGBuffer* Buffer, const void* InitialData, uint32 InitialDataSize);
    
    template<class T>
    TRDGUniformBuffer<T>* CreateUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc(sizeof(T), sizeof(T), EBufferUsageFlags::UniformBuffer);
        TRDGUniformBufferRef<T> Buffer = new TRDGUniformBuffer<T>(Name, Desc);
        Buffer->bTransient = true;
        Buffers.push_back(Buffer);
        return Buffer;
    }

    RDGDescriptorSet* CreateDescriptorSet(std::string Name, RHIDescriptorSetLayout* Layout);



    // Add a graphics pass to the render graph
    template <typename ExecuteLambdaType>
    FRDGPassHandle AddGraphicsPass(
        const RDGPassDesc& PassDesc,
        const RDGRenderTargets& RenderTargets,
        const std::vector<RDGBuffer*>& IndexBuffer,
        const std::vector<RDGBuffer*>& VertexBuffers,
        const std::vector<RDGDescriptorSet*>& PassParameters,
        ExecuteLambdaType&& Executor)
    {
        FRDGPass* Pass = new TRDGLambdaPass<ExecuteLambdaType>(
            Passes.size(), 
            PassDesc, 
            ERHIPipeline::Graphics,
            std::forward<ExecuteLambdaType>(Executor));
        Pass->RenderTargets = RenderTargets;
        Pass->IndexBuffers = IndexBuffer;
        Pass->VertexBuffers = VertexBuffers;
        Pass->DescriptorSets = PassParameters;
        Passes.push_back(Pass);
        SetupParameterPass(Pass);   
        return Pass->Handle;
    }

    // Add a compute pass to the render graph
    template <typename ExecuteLambdaType>
    FRDGPassHandle AddComputePass(
        const RDGPassDesc& PassDesc,
        const std::vector<RDGDescriptorSet*>& PassParameters,
        ExecuteLambdaType&& Executor)
    {
        FRDGPass* Pass = new TRDGLambdaPass<ExecuteLambdaType>(
            Passes.size(), 
            PassDesc, 
            ERHIPipeline::AsyncCompute,
            std::forward<ExecuteLambdaType>(Executor));
        Pass->DescriptorSets = PassParameters;
        Passes.push_back(Pass);
        SetupParameterPass(Pass);
        return Pass->Handle;
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

    template <typename ExecuteLambdaType>
    FRDGPassHandle AddCopyPass(
        const RDGPassDesc& PassDesc,
        RDGBuffer* Source,
        RDGBuffer* Destination,
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

    void SubmitBufferUploads();

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

    struct FUploadedBuffer
    {
        FUploadedBuffer() = default;

        FUploadedBuffer(RDGBuffer* InBuffer, const void* InData, uint64 InDataSize)
            : Buffer(InBuffer)
            , Data(InData)
            , DataSize(InDataSize)
        {

        }

        RDGBuffer* Buffer;
        const void* Data;
        uint64 DataSize;
    };
    std::vector<FUploadedBuffer> UploadedBuffers;

    static std::map<RHIDescriptorSetLayout*, RHIDescriptorSetPools> DescriptorSetPools;

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

    static FRDGPooledTextureRef AllocatePooledRenderTargetRHI(RDGTexture* Texture); 
    static FRDGPooledBufferRef AllocatePooledBufferRHI(RDGBuffer* Buffer);
    void CreateViews(const std::unordered_set<RDGTextureView*>& ViewsToCreate);

    void SetTransientTextureRHI(RDGTexture* Texture, FRHITransientTexture* TransientTexture);
    void SetTransientBufferRHI(RDGBuffer* Buffer, FRHITransientBuffer* TransientBuffer);

    void CompilePassBarriers();
    void CollectPassBarriers();
    void CollectPassBarriers(FRDGPassHandle PassHandle);

    void CollectPassDescriptorSets(FRDGPassHandle PassHandle);

    void ExecuteSerialPass(RHICommandList& RHICmdList, FRDGPass* Pass);

    static void EnumerateTextureAccess(FRDGPass* Pass, const std::function<void(RDGTextureView*,RDGTexture*,RHISamplerState*,ERHIAccess)>& AccessFunction);
    static void EnumerateBufferAccess(FRDGPass* Pass, const std::function<void(RDGBuffer*,ERHIAccess)>& AccessFunction);


};

}