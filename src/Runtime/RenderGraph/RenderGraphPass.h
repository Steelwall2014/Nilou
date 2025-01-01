#pragma once
#include "RenderGraphResources.h"
#include "RenderGraphTransition.h"

namespace nilou {

class RDGDescriptorSet;

struct RDGPassDesc
{
    std::string Name;

    // If this pass should never be culled.
    bool bNeverCull = false;

};

class FRDGPass
{
public:

    FRDGPass(FRDGPassHandle InHandle, const RDGPassDesc& InDesc, ERHIPipeline InPipeline)
        : Handle(InHandle)
        , Desc(InDesc)
        , Name(InDesc.Name)
        , Pipeline(InPipeline)
    {
        
    }

    const std::string Name;
    bool bCulled;
    FRDGPassHandle Handle;
    // Steelwall2014: Instead of using ERDGPassFlags, we use a structure.
    // ERDGPassFlags Flags;
    RDGPassDesc Desc;
    ERHIPipeline Pipeline;

    std::vector<RDGDescriptorSet*> DescriptorSets;
    std::set<FRDGPass*> Producers;
    std::set<FRDGPass*> Consumers;

    std::vector<RHIMemoryBarrier> MemoryBarriers;
    std::vector<RHIBufferMemoryBarrier> BufferBarriers;
    std::vector<RHIImageMemoryBarrier> ImageBarriers;
    std::vector<RHISemaphoreRef> SemaphoresToSignal;
    std::vector<RHISemaphoreRef> SemaphoresToWait;

    void Execute(RHICommandList& RHICmdList);

#if 0   
    // Steelwall2014: These members are related to pass merging and are not used in the project.

	/** The passes which are handling the epilogue / prologue barriers meant for this pass. */
	FRDGPassHandle PrologueBarrierPass; // Steelwall2014: Pointing to the first pass in the merged pass.
	FRDGPassHandle EpilogueBarrierPass; // Steelwall2014: Pointing to the last pass in the merged pass.

	/** Lists of pass parameters scheduled for begin during execution of this pass. */
    // Steelwall2014: What are these?
	std::vector<FRDGPass*> ResourcesToBegin;
	std::vector<FRDGPass*> ResourcesToEnd;
#endif

    struct FTextureState
    {
        FTextureState() = default;

		FTextureState(RDGTexture* InTexture)
			: Texture(InTexture)
		{
			const uint32 SubresourceCount = Texture->GetSubresourceCount();
			Access.resize(SubresourceCount);
		}

		RDGTexture* Texture = nullptr;
		std::vector<ERHIAccess> Access;
		uint16 ReferenceCount = 0;
    };

	struct FBufferState
	{
		FBufferState() = default;

		FBufferState(RDGBuffer* InBuffer)
			: Buffer(InBuffer)
		{}

		RDGBuffer* Buffer = nullptr;
		ERHIAccess Access = ERHIAccess::None;
		uint16 ReferenceCount = 0;
	};

	/** Maps textures / buffers to information on how they are used in the pass. */
	std::vector<FTextureState> TextureStates;
	std::vector<FBufferState> BufferStates;

    FTextureState& FindOrAddTextureState(RDGTexture* Texture)
    {
        if (Texture->LastPass != Handle)
        {
            Texture->LastPass = Handle;
            Texture->PassStateIndex = TextureStates.size();
            return TextureStates.emplace_back(Texture);
        }
        else 
        {
            return TextureStates[Texture->PassStateIndex];
        }
    }

    FBufferState& FindOrAddBufferState(RDGBuffer* Buffer)
    {
        if (Buffer->LastPass != Handle)
        {
            Buffer->LastPass = Handle;
            Buffer->PassStateIndex = BufferStates.size();
            return BufferStates.emplace_back(Buffer);
        }
        else 
        {
            return BufferStates[Buffer->PassStateIndex];
        }
    }

protected:

	//////////////////////////////////////////////////////////////////////////
	//! User Methods to Override

	virtual void ExecuteImpl(RHICommandList& RHICmdList) { }

	//////////////////////////////////////////////////////////////////////////

};

template <typename ExecuteLambdaType>
class TRDGLambdaPass : public FRDGPass
{
public:
    TRDGLambdaPass(FRDGPassHandle InHandle, const RDGPassDesc& InDesc, ERHIPipeline InPipeline, ExecuteLambdaType&& InExecuteLambda)
        : FRDGPass(InHandle, InDesc, InPipeline)
        , ExecuteLambda(std::move(InExecuteLambda))
    {
    }

    ExecuteLambdaType ExecuteLambda;

protected:

	//////////////////////////////////////////////////////////////////////////
	//! User Methods to Override

	virtual void ExecuteImpl(RHICommandList& RHICmdList) override
    {
        ExecuteLambda(RHICmdList);
    }

	//////////////////////////////////////////////////////////////////////////
};

}