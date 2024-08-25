#include "RenderResource.h"
#include "RenderingThread.h"
#include "RenderGraph.h"
#include "RHICommandList.h"

namespace nilou {

    void FRenderResource::InitResource(RenderGraph& Graph)
    {
        if (ListIndex == -1)
        {
            int32 LocalListIndex = -1;
            std::vector<FRenderResource*>& ResourceList = GetResourceList();
			std::vector<int32>& FreeIndicesList = GetFreeIndicesList();
            if (FreeIndicesList.size() > 0)
			{
				LocalListIndex = FreeIndicesList[FreeIndicesList.size()-1]; FreeIndicesList.pop_back();
				ResourceList[LocalListIndex] = this;
			}
            else
			{
				LocalListIndex = ResourceList.size(); 
                ResourceList.push_back(this);
			}

			InitRHI(Graph);
            ListIndex = LocalListIndex;
        }
    }

    void FRenderResource::InitResource()
    {
        if (ListIndex == -1)
        {
            int32 LocalListIndex = -1;
            std::vector<FRenderResource*>& ResourceList = GetResourceList();
			std::vector<int32>& FreeIndicesList = GetFreeIndicesList();
            if (FreeIndicesList.size() > 0)
			{
				LocalListIndex = FreeIndicesList[FreeIndicesList.size()-1]; FreeIndicesList.pop_back();
				ResourceList[LocalListIndex] = this;
			}
            else
			{
				LocalListIndex = ResourceList.size(); 
                ResourceList.push_back(this);
			}

			InitRHI();
            ListIndex = LocalListIndex;
        }
    }

    void FRenderResource::ReleaseResource()
    {
        if (ListIndex != -1)
        {
            ReleaseRHI();
			std::vector<FRenderResource*>& ResourceList = GetResourceList();
			std::vector<int32>& FreeIndicesList = GetFreeIndicesList();
			ResourceList[ListIndex] = nullptr;
			FreeIndicesList.push_back(ListIndex);

			ListIndex = -1;

        }
    }

    void FRenderResource::UpdateRHI()
    {
        if (IsInitialized())
        {
            ReleaseRHI();
            InitRHI(FRenderingThread::GetRenderGraph());
        }
    }

    void BeginInitResource_Internal(FRenderResource* Resource, const char *file, int line)
    {
        // ENQUEUE_RENDER_COMMAND(BeginInitResource)(
        //    [Resource, file, line](FDynamicRHI *DynamicRHI)
        //    {
        //         const char *debug_file = file;
        //         int debug_line = line;
        //         Resource->InitResource();
        //    });
        Resource->InitResource(FRenderingThread::GetRenderGraph());
    }

    void BeginReleaseResource_Internal(FRenderResource* Resource, const char *file, int line)
    {
        if (!Resource->IsInitialized()) return;
        ENQUEUE_RENDER_COMMAND(BeginReleaseResource)(
           [Resource, file, line](FDynamicRHI *DynamicRHI)
           {
                const char *debug_file = file;
                int debug_line = line;
                Resource->ReleaseResource();
           });
    }

    std::vector<int32>& GetFreeIndicesList()
    {
        static std::vector<int32> FreeIndicesList;
        return FreeIndicesList;
    }

    std::vector<FRenderResource*>& FRenderResource::GetResourceList()
    {
        static std::vector<FRenderResource*> RenderResourceList;
        return RenderResourceList;
    }

    RHIBuffer* FVertexBuffer::GetRHI() const
    {
        return VertexBufferRDG->Resolve();
    }

    RHIBuffer* FIndexBuffer::GetRHI() const
    {
        return IndexBufferRDG->Resolve();
    }

}