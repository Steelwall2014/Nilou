#include "LineBatchComponent.h"

#include "Common/Scene.h"

namespace nilou {

    void ULineBatchComponent::DrawLines(const std::vector<FBatchedLine> &InLines)
    {
        for (const FBatchedLine &Line : InLines)
            BatchedLines.push_back(Line);
        
        MarkRenderStateDirty();
    }

    class FLineBatchSceneProxy : public FPrimitiveSceneProxy
    {
    public:

        FLineBatchSceneProxy(ULineBatchComponent *InComponent)
            : FPrimitiveSceneProxy(InComponent)
        {
            BatchedLines = InComponent->BatchedLines;
        }

        virtual void GetDynamicMeshElements(const std::vector<FViewSceneInfo*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                for (int i = 0; i < BatchedLines.size(); i += 1)
                {
                    Views[ViewIndex]->PDI->DrawLine(BatchedLines[i]);
                }
            }
        }

    private:
        std::vector<FBatchedLine> BatchedLines;
    
    };

    FPrimitiveSceneProxy *ULineBatchComponent::CreateSceneProxy()
    {
        return new FLineBatchSceneProxy(this);
    }

}