#pragma once

#include "RHIResources.h"
#include "Common/Log.h"

namespace nilou {

    int FRHIGraphicsPipelineState::GetBaseIndexByName(EPipelineStage PipelineStage, const std::string &Name)
    {
        FRHIDescriptorSet &DescriptorSet = PipelineLayout.DescriptorSets[PipelineStage];
        auto iter = DescriptorSet.Bindings.find(Name);
        if (iter != DescriptorSet.Bindings.end())
            return iter->second.BindingPoint;
        else
        {
            NILOU_LOG(Error, "Shader Parameter {} Not found", Name.c_str())
            return -1;
        }
    }
}