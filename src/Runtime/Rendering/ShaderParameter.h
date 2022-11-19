#pragma once

#include <string>
// #include "UniformBuffer.h"
#include "Platform.h"
// #include "RHIResources.h"

// #define DECLARE_SHADER_INPUT(Type, Name) \
//     TShaderResourceParameter<Type> Name = TShaderResourceParameter<Type>(#Name);

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
    // enum class EShaderInputType
    // {
    //     SPT_Sampler,
    //     SPT_Uniform,
    //     SPT_UniformBuffer,
    // };

    // class FShaderResourceParameter
    // {
    // protected:
    //     std::string ParameterName;
    //     EShaderInputType ShaderInputType;
    //     FShaderResourceParameter(const char *InParameterName)
    //         : ParameterName(InParameterName) {}
    // };

    // template<class InShaderParamType>
    // class TShaderResourceParameter : public FShaderResourceParameter
    // {
    // public:
    //     TShaderResourceParameter(const char *InParameterName) 
    //         : FShaderResourceParameter(InParameterName)
    //     {}
    //     void Set(const InShaderParamType &Value);
    // };

    // template<class ResourceType>
    // class FShaderResourceParameter : public FBaseShaderParameter
    // {
    // protected:
    //     std::string ParameterName;
    //     uint32 BindingPoint;

    // };

    // template<>
    // class FShaderResourceParameter<int32>
    // {
    // public:
    //     void Set(int32 Value);
    // };
    // using FSRPInt = FShaderResourceParameter<int32>;

    // template<>
    // class FShaderResourceParameter<FRHISampler2D>
    // {
    // public:
    //     void Set(FRHISampler2D *Value);
    // };
    // using FSRPSampler2D = FShaderResourceParameter<FRHISampler2D>;

    // template<>
    // class FShaderResourceParameter<FRHISampler3D>
    // {
    // public:
    //     void Set(FRHISampler3D *Value);
    // };
    // using FSRPSampler3D = FShaderResourceParameter<FRHISampler3D>;

    // template<>
    // class FShaderResourceParameter<RHIUniformBuffer>
    // {
    // public:
    //     void Set(RHIUniformBuffer *Value);
    // };
    // using FSRPUniformBuffer = FShaderResourceParameter<RHIUniformBuffer>;

}