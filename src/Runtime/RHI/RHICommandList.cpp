#include "RHICommandList.h"
#include "DynamicRHI.h"

namespace nilou {

RHICommandListExecutor::RHICommandListExecutor()
{
    RHICmdList = RHICreateCommandList();
}

RHICommandListExecutor::~RHICommandListExecutor()
{

}

void RHICommandListExecutor::Submit(const std::vector<RHISemaphoreRef>& SemaphoresToWait, const std::vector<RHISemaphoreRef>& SemaphoresToSignal)
{
    RHISubmitCommandList(RHICmdList, SemaphoresToWait, SemaphoresToSignal);
}

}