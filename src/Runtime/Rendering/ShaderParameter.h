#pragma once

#include <string>
#include "Platform.h"

namespace nilou {


    enum class EShaderParameterType
    {
        SPT_Sampler,
        // SPT_Uniform,
        SPT_UniformBuffer,
        // SPT_ShaderStructureBuffer,
        SPT_Image,
    };

    class FShaderParameterInfo
    {
    public:
        std::string ParameterName;
        EShaderParameterType ParameterType;
        int16 BindingPoint;
        // uint16 Size;

        FShaderParameterInfo()
            : ParameterName("")
            , BindingPoint(-1)
            , ParameterType(EShaderParameterType::SPT_UniformBuffer)
        {
        }

        FShaderParameterInfo(const std::string &InParameterName, int16 InBindingPoint, EShaderParameterType InParameterType/*, uint16 InSize*/)
            : ParameterName(InParameterName)
            , BindingPoint(InBindingPoint)
            , ParameterType(InParameterType)
        {
        }

        bool operator<(const FShaderParameterInfo& Other) const
        {
            return ParameterName < Other.ParameterName;
        }
        
        // inline bool operator==(const FShaderParameterInfo& Rhs) const
        // {
        //     return BindingPoint == Rhs.BindingPoint
        //         && Size == Rhs.Size;
        // }

        // inline bool operator<(const FShaderParameterInfo& Rhs) const
        // {
        //     return BindingPoint < Rhs.BindingPoint;
        // }
    };

}