#include "HAL/PlatformMisc.h"
#include "RHIDefinitions.h"
#include "RenderGraphPass.h"
#include "RHICommandList.h"

namespace nilou {

namespace
{
	FColor RDGPassDebugColor(ERHIPipeline InPipeline)
	{
		if (EnumHasAnyFlags(InPipeline, ERHIPipeline::Graphics))
		{
			return FColor(60, 140, 230);
		}
		if (EnumHasAnyFlags(InPipeline, ERHIPipeline::AsyncCompute))
		{
			return FColor(230, 160, 40);
		}
		if (EnumHasAnyFlags(InPipeline, ERHIPipeline::Copy))
		{
			return FColor(90, 210, 120);
		}
		return FColor::Red;
	}
}

void FRDGPass::Execute(RHICommandList& RHICmdList)
{
    std::string RDGPassName = GetName();
    RDGPassName = RDGPassName.size() > 0 ? RDGPassName : "None";
    SCOPED_NAMED_EVENT(RDGPassName, FColor::Red);

	RHICmdList.PushEvent(RDGPassName.c_str(), RDGPassDebugColor(Pipeline));
    ExecuteImpl(RHICmdList);
	RHICmdList.PopEvent();
}

}
