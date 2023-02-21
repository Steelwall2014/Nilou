#include "RenderResource.h"
#include "RenderingThread.h"

namespace nilou {

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

    void BeginInitResource(FRenderResource* Resource)
    {
        ENQUEUE_RENDER_COMMAND(BeginInitResource)(
           [Resource](FDynamicRHI *DynamicRHI)
           {
                Resource->InitResource();
           });
    }

    void BeginReleaseResource(FRenderResource* Resource)
    {
        ENQUEUE_RENDER_COMMAND(BeginReleaseResource)(
           [Resource](FDynamicRHI *DynamicRHI)
           {
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

}