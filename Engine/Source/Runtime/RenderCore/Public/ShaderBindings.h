#pragma once

#include <string>
#include <utility>
#include <vector>
#include <variant>

#include "UniformBuffer.h"
#include "RHIDefinitions.h"
#include "Templates/TypeTraits.h"
namespace nilou {

    // class FUniformValue
    // {
    // public:
    //     FUniformValue()
    //     {
    //     }
    //     FUniformValue(const FUniformValue& Other)
    //     {
    //         Value = Other.Value;
    //     }
    //     FUniformValue(int32 InValue)
    //     {
    //         Value = InValue;
    //     }
    //     FUniformValue(float InValue)
    //     {
    //         Value = InValue;
    //     }
    //     FUniformValue(uint32 InValue)
    //     {
    //         Value = InValue;
    //     }
    //     std::variant<int32, float, uint32> Value;
    // };

    // class FInputShaderBindings
    // {
    // public:
    //     template<class T>
    //     void SetElementShaderBinding(const std::string &Name, T *Resource)
    //     {
    //         if (Resource == nullptr)
    //             return;
    //         static_assert(
    //             std::is_same<T, RHIUniformBuffer>::value || std::is_same<T, FRHISampler>::value || std::is_same<T, RHIBuffer>::value, 
    //             "Resource type must be RHIUniformBuffer, FRHISampler or RHIBuffer");
    //         if constexpr (std::is_same<T, RHIUniformBuffer>::value)
    //             UniformBuffers[Name] = Resource;
    //         else if constexpr (std::is_same<T, FRHISampler>::value)
    //             Samplers[Name] = Resource;
    //         else if constexpr (std::is_same<T, RHIBuffer>::value)
    //             Buffers[Name] = Resource;
    //     }
    //     template<class T>
    //     T *GetElementShaderBinding(const std::string &Name)
    //     {
    //         static_assert(
    //             std::is_same<T, RHIUniformBuffer>::value || 
    //             std::is_same<T, FRHISampler>::value || 
    //             std::is_same<T, RHIBuffer>::value || 
    //             "Resource type must be RHIUniformBuffer, FRHISampler or RHIBuffer");
    //         if constexpr (std::is_same<T, RHIUniformBuffer>::value)
    //             return UniformBuffers[Name];
    //         else if constexpr (std::is_same<T, FRHISampler>::value)
    //             return Samplers[Name];
    //         else if constexpr (std::is_same<T, RHIBuffer>::value)
    //             return Buffers[Name];
    //         return nullptr;
    //     }
    //     // template<typename T>
    //     // void SetUniformShaderBinding(const std::string &Name, T Value)
    //     // {
    //     //     Uniforms.insert({Name, Value});;
    //     // }
    //     // std::optional<FUniformValue> GetUniformShaderBinding(const std::string &Name)
    //     // {
    //     //     if (Uniforms.contains(Name))
    //     //         return Uniforms.find(Name)->second;
    //     //     return std::nullopt;
    //     // }
    
    // private:
	//     std::map<std::string, RHIUniformBuffer *> UniformBuffers;
	//     std::map<std::string, FRHISampler *> Samplers;
	//     std::map<std::string, RHIBuffer *> Buffers;
    //     // std::map<std::string, FUniformValue> Uniforms;

    // };
}