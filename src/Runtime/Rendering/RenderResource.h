#pragma once

#include "RHIResources.h"
#include "glm/glm.hpp"
#include <vector>

namespace nilou {

    class FRenderResource
    {
    public:
        virtual void InitRHI() { bRHIInitialized = true; }
        virtual void ReleaseRHI() { bRHIInitialized = false; }
        virtual void InitResource();
        virtual void ReleaseResource();
        virtual ~FRenderResource() { ReleaseResource(); }
        bool IsInitialized() { return bRHIInitialized; }
        void UpdateRHI()
        {
            if(IsInitialized())
            {
                ReleaseRHI();
                InitRHI();
            }
        }
        static std::vector<FRenderResource*>& GetResourceList();

    private:
        int32 ListIndex = -1;
        bool bRHIInitialized = false;
    };

    class FVertexBuffer : public FRenderResource
    {
    public:
        RHIBufferRef VertexBufferRHI;
        
        virtual void ReleaseRHI() { VertexBufferRHI = nullptr; }
    };

    class FIndexBuffer : public FRenderResource
    {
    public:
        RHIBufferRef IndexBufferRHI;
        uint32 Stride;
        uint32 NumIndices;
        
        virtual void ReleaseRHI() { IndexBufferRHI = nullptr; }
    };

    #define BeginInitResource(Resource) BeginInitResource_Internal(Resource, __FILE__, __LINE__)

    #define BeginReleaseResource(Resource) BeginReleaseResource_Internal(Resource, __FILE__, __LINE__)

    void BeginInitResource_Internal(FRenderResource* Resource, const char *file, int line);
    void BeginReleaseResource_Internal(FRenderResource* Resource, const char *file, int line);

    std::vector<int32>& GetFreeIndicesList();
}