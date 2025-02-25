#pragma once

#include <vector>
#include "DynamicRHI.h"
#include "RenderResource.h"
#include "RenderGraph.h"

namespace nilou {

    template<class VertexType>
    class FStaticMeshVertexBuffer : public FVertexBuffer
    {
    public:

        FStaticMeshVertexBuffer()
            : Data(nullptr)
        { }

        virtual ~FStaticMeshVertexBuffer()
        {
            CleanUp();
        }

        void Init(uint32 InNumVertices);

        void Init(const std::vector<VertexType>& InVertices);

        void Init(void *InData, uint32 Stride, uint32 InNumVertices);

        void CleanUp();

        virtual void InitRHI(RenderGraph& Graph) override;

        inline uint32 GetStride() const
        {
            return Stride;
        }

        inline uint32 GetNumVertices() const
        {
            return NumVertices;
        }

        void* GetVertexData() { return Data; }

        const void* GetVertexData() const { return Data; }

        VertexType GetVertexValue(int index)
        {
            VertexType *ptr = reinterpret_cast<VertexType *>(Data) + index;
            return *ptr;
        }

        void SetVertexValue(int index, const VertexType &Value)
        {
            VertexType *ptr = reinterpret_cast<VertexType *>(Data) + index;
            *ptr = Value;
        }

        void BindToVertexFactoryData(FVertexStreamComponent &OutStreamComponent, uint32 Offset = 0)
        {
            if (Stride == 0 || NumVertices == 0)
                return;
            if constexpr (std::is_same<VertexType, vec2>::value)
            {
                OutStreamComponent = FVertexStreamComponent(
                    this, 
                    Offset, 
                    GetStride(), 
                    EVertexElementType::VET_Float2);
            }
            else if constexpr (std::is_same<VertexType, vec3>::value)
            {
                OutStreamComponent = FVertexStreamComponent(
                    this, 
                    Offset, 
                    GetStride(), 
                    EVertexElementType::VET_Float3);
            }
            else if constexpr (std::is_same<VertexType, vec4>::value)
            {
                OutStreamComponent = FVertexStreamComponent(
                    this, 
                    Offset, 
                    GetStride(), 
                    EVertexElementType::VET_Float4);
            }
        }

    private:

        uint8 *Data;

        // bool bNeedsCPUAccess = true;

    };

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::Init(uint32 InNumVertices)
    {
        Stride = sizeof(VertexType);
        NumVertices = InNumVertices;
        if (NumVertices > 0)
            Data = (uint8 *)std::realloc(Data, NumVertices * sizeof(VertexType));
    }

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::Init(const std::vector<VertexType>& InPositions)
    {
        Stride = sizeof(VertexType);
        NumVertices = InPositions.size();
        if (NumVertices > 0)
        {
            Data = (uint8 *)std::realloc(Data, NumVertices * sizeof(VertexType));
            memcpy(Data, InPositions.data(), NumVertices * sizeof(VertexType));
        }
    }

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::Init(void *InData, uint32 InStride, uint32 InNumVertices)
    {
        Stride = InStride;
        NumVertices = InNumVertices;
        if (NumVertices > 0)
        {
            Data = (uint8 *)std::realloc(Data, NumVertices * InStride);
            memcpy(Data, InData, NumVertices * InStride);
        }
    }

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::CleanUp()
    {
        if (Data)
        {
            delete[] Data;
            Data = nullptr;
        }
    }

    template<class VertexType>
    void FStaticMeshVertexBuffer<VertexType>::InitRHI(RenderGraph& Graph)
    {
        FRenderResource::InitRHI(Graph);
        RDGBufferDesc Desc;
        Desc.BytesPerElement = Stride;
        Desc.NumElements = NumVertices;
        VertexBufferRDG = RenderGraph::CreateExternalBuffer("", Desc);
        // Graph.AddUploadPass(VertexBufferRDG, Data, Desc.GetSize());
    }

}