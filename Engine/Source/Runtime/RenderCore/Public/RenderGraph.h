#pragma once
#include <vector>

#include "Shader.h"
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "RenderGraphDescriptorSet.h"
#include "RenderGraphParameterBlock.h"
#include "RenderGraphPass.h"
#include "UniformBuffer.h"

namespace nilou {

class FDynamicRHI;
class RHICommandList;
class IRHITransientResourceAllocator;

class RENDERCORE_API RenderGraph
{
public:

    friend class RDGBuilder;

    RenderGraph();
    ~RenderGraph();

    RenderGraph(const RenderGraph&) = delete;
    RenderGraph& operator=(const RenderGraph&) = delete;
    RenderGraph& operator=(RenderGraph&&) = default;

    void BeginFrame();
    void EndFrame();

    RDGTexture* GetSwapChainTexture() const { return SwapChainTexture; }

    static RDGTextureRef CreatePooledTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    static RDGTextureViewRef CreatePooledTextureView(const std::string& Name, RDGTexture* Texture, const RDGTextureViewDesc& TextureViewDesc);

    static RDGTextureViewRef CreatePooledTextureView(RDGTexture* Texture);

    static RDGBufferRef CreatePooledBuffer(const std::string& Name, const RDGBufferDesc& Desc);
    
    template<class T>
    static TRDGUniformBufferRef<T> CreatePooledUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc(sizeof(T), sizeof(T), EBufferUsageFlags::UniformBuffer);
        TRDGUniformBufferRef<T> Buffer = TRefCountPtr(new TRDGUniformBuffer<T>(Name, Desc));
        Buffer->bTransient = false;
        return Buffer;
    }

    static RDGDescriptorSetRef CreatePooledDescriptorSet(std::string Name, RHIDescriptorSetLayout* Layout);

    RDGTexture* RegisterExternalTexture(const std::string& Name, RHITexture* TextureRHI);

    RDGTexture* CreateTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    RDGTextureView* CreateTextureView(const std::string& Name, RDGTexture* Texture, const RDGTextureViewDesc& TextureViewDesc);

    RDGTextureView* CreateTextureView(RDGTexture* Texture);

    RDGBuffer* CreateBuffer(const std::string& Name, const RDGBufferDesc& Desc);

    void QueueBufferUpload(RDGBuffer* Buffer, const void* InitialData, uint32 InitialDataSize);
    
    template<class T>
    TRDGUniformBuffer<T>* CreateUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc(sizeof(T), sizeof(T), EBufferUsageFlags::UniformBuffer);
        TRDGUniformBufferRef<T> Buffer = TRefCountPtr(new TRDGUniformBuffer<T>(Name, Desc));
        Buffer->bTransient = true;
        Buffers.push_back(Buffer);
        return Buffer.GetReference();
    }

    RDGDescriptorSet* CreateDescriptorSet(std::string Name, RHIDescriptorSetLayout* Layout);

    // Creates a transient TParameterBlock owned by this RenderGraph.
    // The block (and its DS) is freed during CleanUp(). The caller's ParamBlock
    // must remain in scope until Execute() so QueueBufferUpload data stays valid.
    // Call UpdateParameterBlock whenever the parameter data changes.
    template <template<EShaderDataLayout DataLayout> typename T>
    TParameterBlock<T>* CreateParameterBlock(const std::string& Name)
    {
        TParameterBlock<T>* Block = new TParameterBlock<T>();
        Block->Name = Name;
        Block->bTransient = true;
        ParamBlockDeleters.emplace_back([Block]() { delete Block; });
        return Block;
    }

    // Creates a pooled TParameterBlock whose lifetime is managed by the caller via
    // TParameterBlockRef. The embedded DS persists as long as the ref is held.
    // Call UpdateParameterBlock whenever the parameter data changes.
    template <template<EShaderDataLayout DataLayout> typename T>
    static TParameterBlockRef<T> CreatePooledParameterBlock(const std::string& Name)
    {
        TParameterBlockRef<T> Block = TRefCountPtr(new TParameterBlock<T>());
        Block->Name = Name;
        Block->bTransient = false;
        return Block;
    }

    // Materializes (or re-materializes) the descriptor set for a parameter block.
    // The caller must invoke this explicitly whenever fields change.
    template <template<EShaderDataLayout DataLayout> typename T>
    void UpdateParameterBlock(TParameterBlock<T>* Block)
    {
        FShaderParametersMetadata2* Metadata = GetShaderParametersMetadata<T>();
        RHIDescriptorSetLayout* DescriptorSetLayout = Metadata->DescriptorSetLayout.GetReference();

        if (!Block->DescriptorSet)
        {
            if (Block->bTransient)
            {
                Block->DescriptorSet = CreateDescriptorSet(Block->Name, DescriptorSetLayout);
            }
            else
            {
                RDGDescriptorSetRef DescriptorSet = CreatePooledDescriptorSet(Block->Name, DescriptorSetLayout);
                Block->DescriptorSet = DescriptorSet.GetReference();
                DescriptorSet->AddRef();
            }
        }

        for (const FShaderParametersMetadata2::FMember& Member : Metadata->Members)
        {
            if (Member.Name == "AutomaticallyIntroducedUniformBuffer")
            {
                uint8* OpaqueBase = reinterpret_cast<uint8*>(&Block->GetOpaqueFields());
                RDGBuffer** pAutoUB = reinterpret_cast<RDGBuffer**>(OpaqueBase + Member.Offset);
                if (!*pAutoUB)
                {
                    if (Block->bTransient)
                    {
                        *pAutoUB = CreateUniformBuffer<T<Std140Layout>>(Block->Name + "_AutoUB");
                    }
                    else
                    {
                        RDGBufferRef AutoUB = CreatePooledUniformBuffer(Block->Name + "_AutoUB");
                        *pAutoUB = AutoUB.GetReference();
                        AutoUB->AddRef();
                    }
                }
                T<Std140Layout>& NonOpaqueFields = Block->GetNonOpaqueFields();
                QueueBufferUpload(*pAutoUB, reinterpret_cast<const void*>(&NonOpaqueFields), sizeof(NonOpaqueFields));
                break;
            }
        }
    }
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

    // Add a graphics pass to the render graph
    template <typename SetupLambdaType, typename ExecuteLambdaType>
    FRDGPassHandle AddGraphicsPass(
        const RDGPassDesc& PassDesc,
        const RDGRenderTargets& RenderTargets,
        const std::vector<RDGBuffer*>& IndexBuffer,
        const std::vector<RDGBuffer*>& VertexBuffers,
        SetupLambdaType&& Setup,
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
        Setup(Pass);
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

    // Single-parameter-block overload: auto-materializes transient blocks;
    // pooled blocks must have been materialized by the caller via UpdateParameterBlock.
    template <typename SetupLambdaType, typename ExecuteLambdaType>
    FRDGPassHandle AddComputePass(
        const RDGPassDesc& PassDesc,
        SetupLambdaType&& Setup,
        ExecuteLambdaType&& Executor)
    {
        FRDGPass* Pass = new TRDGLambdaPass<ExecuteLambdaType>(
            Passes.size(), 
            PassDesc, 
            ERHIPipeline::AsyncCompute,
            std::forward<ExecuteLambdaType>(Executor));
        Setup(Pass);
        Passes.push_back(Pass);
        SetupParameterPass(Pass);
        return Pass->Handle;
    }

    struct PassBuilder
    {
        PassBuilder(RenderGraph& InGraph, FRDGPass* InPass)
            : Graph(InGraph)
            , Pass(InPass)
        {

        }
        RenderGraph& Graph;
        FRDGPass* Pass;

        template <typename T>
        void AddParameterBlock(const T& ParamBlock)
        {

        }
    };

    // Add a compute pass to the render graph
    template <typename ExecuteLambdaType>
    FRDGPassHandle AddComputePass(
        const RDGPassDesc& PassDesc,
        std::function<void(PassBuilder&)> BuildPass,
        ExecuteLambdaType&& Executor)
    {
        FRDGPass* Pass = new TRDGLambdaPass<ExecuteLambdaType>(
            Passes.size(), 
            PassDesc, 
            ERHIPipeline::AsyncCompute,
            std::forward<ExecuteLambdaType>(Executor));
        PassBuilder Builder(*this, Pass);
        BuildPass(Builder);
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

    RDGTexture* SwapChainTexture = nullptr;

	/** The epilogue and prologue passes are sentinels that are used to simplify graph logic around barriers
	*  and traversal. The prologue pass is used exclusively for barriers before the graph executes, while the
	*  epilogue pass is used for resource extraction barriers--a property that also makes it the main root of
	*  the graph for culling purposes. The epilogue pass is added to the very end of the pass array for traversal
	*  purposes. The prologue does not need to participate in any graph traversal behavior.
	*/
	FRDGPass* ProloguePass = nullptr;
	FRDGPass* EpiloguePass = nullptr;
    FRDGPass* PresentPass = nullptr;

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

    // Deleters for transient TParameterBlock instances; invoked during CleanUp().
    std::vector<std::function<void()>> ParamBlockDeleters;

	/** Registry of graph objects. */
    std::vector<FRDGPass*> Passes;
    std::vector<RDGTextureRef> Textures;
    std::vector<RDGTextureViewRef> TextureViews;
    std::vector<RDGBufferRef> Buffers;
    std::vector<RDGDescriptorSetRef> DescriptorSets;

    // Note by Steelwall2014: 
    // Although the lifetime of the pooled resources is not managed by the RenderGraph,
    // the RenderGraph still needs to borrow the ownership of the pooled resources to 
    // make sure the pooled resources are not destroyed before the ~RenderGraph().
    std::vector<RDGTextureRef> PooledTextures;
    std::vector<RDGTextureViewRef> PooledTextureViews;
    std::vector<RDGBufferRef> PooledBuffers;
    std::vector<RDGDescriptorSetRef> PooledDescriptorSets;

    std::unordered_map<RHITexture*, RDGTexture*> ExternalTextures;

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

    template <template<EShaderDataLayout DataLayout> typename T>
    static void SetupDescriptorSet(const TParameterBlock<T>* ParamBlock, RDGDescriptorSet* DescriptorSet)
    {
        FShaderParametersMetadata2* Metadata = GetShaderParametersMetadata<T>();
        const T<EShaderDataLayout::Opaque>& OpaqueFields = ParamBlock->GetOpaqueFields();
        const uint8* Base = reinterpret_cast<const uint8*>(&OpaqueFields);
        for (const FShaderParametersMetadata2::FMember& Member : Metadata->Members)
        {
            switch (Member.BaseType)
            {
            case EUniformBufferBaseType2::Buffer:
            {
                RDGBuffer* Buffer = *reinterpret_cast<RDGBuffer* const*>(Base + Member.Offset);
                DescriptorSet->SetBuffer(Member.BindingIndex, Buffer);
                break;
            }
            case EUniformBufferBaseType2::Texture:
            {
                RDGTextureView* TextureView = *reinterpret_cast<RDGTextureView* const*>(Base + Member.Offset);
                DescriptorSet->SetTexture(Member.BindingIndex, TextureView);
                break;
            }
            case EUniformBufferBaseType2::TextureSampler:
            {
                RDGCombinedTextureSampler Sampler = *reinterpret_cast<const RDGCombinedTextureSampler*>(Base + Member.Offset);
                DescriptorSet->SetCombinedTextureSampler(Member.BindingIndex, Sampler.TextureView, Sampler.SamplerState);
                break;
            }
            case EUniformBufferBaseType2::Sampler:
            {
                RHISamplerState* SamplerState = *reinterpret_cast<RHISamplerState* const*>(Base + Member.Offset);
                DescriptorSet->SetSamplerState(Member.BindingIndex, SamplerState);
                break;
            }
            default:
                break;
            }
        }
    }

};

}