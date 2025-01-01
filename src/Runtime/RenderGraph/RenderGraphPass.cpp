#include "PlatformMisc.h"
#include "RenderGraphPass.h"
#include "RHICommandList.h"

namespace nilou {

void FRDGPass::Execute(RHICommandList& RHICmdList)
{
    std::string RDGPassName = Name;
    RDGPassName = RDGPassName.size() > 0 ? RDGPassName : "None";
    SCOPED_NAMED_EVENT(RDGPassName, FColor::Red);
    
    ExecuteImpl(RHICmdList);
}

}
