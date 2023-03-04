#pragma once

#include <string>
#include <utility>
#include <vector>

#include "UniformBuffer.h"
#include "RHIDefinitions.h"
#include "Templates/TypeTraits.h"
namespace nilou {

    class FInputShaderBindings
    {
    public:
        template<class T>
        void SetElementShaderBinding(const std::string &Name, T *Resource)
        {
            static_assert(
                TIsDerivedFrom<T, FUniformBuffer>::Value || std::is_same<T, FRHISampler>::value || std::is_same<T, RHIBuffer>::value, 
                "Resource type must be FUniformBuffer, FRHISampler or RHIBuffer");
            if constexpr (TIsDerivedFrom<T, FUniformBuffer>::Value)
                UniformBuffers[Name] = Resource;
            else if constexpr (std::is_same<T, FRHISampler>::value)
                Samplers[Name] = Resource;
            else if constexpr (std::is_same<T, RHIBuffer>::value)
                Buffers[Name] = Resource;
        }
        template<class T>
        T *GetElementShaderBinding(const std::string &Name)
        {
            static_assert(
                TIsDerivedFrom<T, FUniformBuffer>::Value || std::is_same<T, FRHISampler>::value || std::is_same<T, RHIBuffer>::value, 
                "Resource type must be FUniformBuffer, FRHISampler or RHIBuffer");
            if constexpr (TIsDerivedFrom<T, FUniformBuffer>::Value)
                return UniformBuffers[Name];
            else if constexpr (std::is_same<T, FRHISampler>::value)
                return Samplers[Name];
            else if constexpr (std::is_same<T, RHIBuffer>::value)
                return Buffers[Name];
        }
    
    private:
	    std::map<std::string, FUniformBuffer *> UniformBuffers;
	    std::map<std::string, FRHISampler *> Samplers;
	    std::map<std::string, RHIBuffer *> Buffers;

    };
}