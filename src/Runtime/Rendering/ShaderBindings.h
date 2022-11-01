#pragma once

#include <string>
#include <utility>
#include <vector>

#include "UniformBuffer.h"
#include "RHIDefinitions.h"
#include "Templates/TypeTraits.h"
namespace nilou {

    class FElementShaderBindings
    {
    public:
        template<class T>
        void SetElementShaderBinding(const std::string &Name, T *Resource)
        {
            static_assert(
                TIsDerivedFrom<T, FUniformBuffer>::Value || std::is_same<T, FRHISampler>::value, 
                "Resource type must be FUniformBuffer or FRHISampler");
            if constexpr (TIsDerivedFrom<T, FUniformBuffer>::Value)
                UniformBuffers[Name] = Resource;
            else if constexpr (std::is_same<T, FRHISampler>::value)
                Samplers[Name] = Resource;
        }
        template<class T>
        T *GetElementShaderBinding(const std::string &Name)
        {
            static_assert(
                TIsDerivedFrom<T, FUniformBuffer>::Value || std::is_same<T, FRHISampler>::value, 
                "Resource type must be FUniformBuffer or FRHISampler");
            if constexpr (TIsDerivedFrom<T, FUniformBuffer>::Value)
                return UniformBuffers[Name];
            else if constexpr (std::is_same<T, FRHISampler>::value)
                return Samplers[Name];
        }
    
    private:
	    std::map<std::string, FUniformBuffer *> UniformBuffers;
	    std::map<std::string, FRHISampler *> Samplers;

    };
}