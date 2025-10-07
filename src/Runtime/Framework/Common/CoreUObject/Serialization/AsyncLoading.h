#include "Thread.h"
#include "Templates/IoPriorityQueue.h"
#include "Common/Containers/Map.h"
#include "Common/CoreUObject/Package.h"

namespace nilou {

class FAsyncPackage;
class FAsyncLoadEventQueue;

struct FPackageRequest
{
    std::string PackageName;
    int32 Priority;
    int32 RequestId;
};

struct FAsyncPackageDesc
{
    TArray<int32> RequestIds;

    int32 Priority;

    std::string PackageName;
};

/**
 * Event node.
 */
enum EEventLoadNode : uint8
{
	Package_ProcessSummary,
	Package_DependenciesReady,
	Package_LoadImports,
	Package_LoadExports,
	Package_ExportsSerialized,
	Package_NumPhases,

	ExportBundle_Process = 0,
	ExportBundle_PostLoad,
	ExportBundle_DeferredPostLoad,
	ExportBundle_NumPhases,
};

enum class EAsyncPackageLoadingState : uint8
{
	NewPackage,
	WaitingForIo,
	ProcessPackageSummary,
	WaitingForDependencies,
	DependenciesReady,
// This is the path taken by LinkerLoad packages
    LoadImports,
    LoadExports,
// This is the path taken by Runtime/cooked packages
	ProcessExportBundles,
// Both LinkerLoad and Cooked packages should converge at this point
	WaitingForExternalReads,
	ExportsDone,
	PostLoad,
	DeferredPostLoad,
	DeferredPostLoadDone,
	Finalize,
	PostLoadInstances,
	CreateClusters,
	Complete,
	DeferredDelete,
};

struct FAsyncLoadEventSpec
{
    using FAsyncLoadEventFunc = std::function<void(FAsyncPackage*)>;
    FAsyncLoadEventFunc Func = nullptr;
    FAsyncLoadEventQueue* EventQueue = nullptr;
    bool bExecuteImmediately = false;
};

struct FEventLoadNode
{
    FEventLoadNode* Next = nullptr;
    FEventLoadNode* Prev = nullptr;
    int32 Priority = 0;

    const FAsyncLoadEventSpec* Spec = nullptr;
    FAsyncPackage* Package = nullptr;
    FEventLoadNode(const FAsyncLoadEventSpec* InSpec, FAsyncPackage* InPackage, int32 InPriority)
        : Spec(InSpec), Package(InPackage), Priority(InPriority)
    {

    }

    void Fire();

    void Execute();

    void ReleaseBarrier()
    {
        Ncheck(BarrierCount > 0);
        if (--BarrierCount == 0)
        {
            Fire();
        }
    }

    void AddBarrier()
    {
        ++BarrierCount;
    }

private:
	std::atomic<int32> BarrierCount { 0 };
};

struct FAsyncPackageData
{
    TArray<std::weak_ptr<FAsyncPackage>> ImportedAsyncPackages;
};

struct FAsyncPackage : public std::enable_shared_from_this<FAsyncPackage>
{
    FAsyncPackageDesc Desc;

	uint8 PackageNodesMemory[Package_NumPhases * sizeof(FEventLoadNode)];
    TArrayView<FEventLoadNode> PackageNodes;

    NPackage* LinkerRoot = nullptr;

    EAsyncPackageLoadingState AsyncPackageLoadingState = EAsyncPackageLoadingState::NewPackage;

    class FAsyncLoadingThread& AsyncLoadingThread;

    TArray<FObjectExport> ObjectExports;
    TArray<FObjectImport> ObjectImports;

    EAsyncLoadingResult LoadResult = EAsyncLoadingResult::Succeeded;

    FAsyncPackage(const FAsyncPackageDesc& InDesc, FAsyncLoadingThread& InAsyncLoadingThread, FAsyncLoadEventSpec* EventSpecs)
        : Desc(InDesc)
        , AsyncLoadingThread(InAsyncLoadingThread)
    {
        FEventLoadNode* Node = reinterpret_cast<FEventLoadNode*>(PackageNodesMemory);
        for (int32 Phase = 0; Phase < EEventLoadNode::Package_NumPhases; ++Phase)
        {
            new (Node + Phase) FEventLoadNode(&EventSpecs[Phase], this, Desc.Priority);
        }
        PackageNodes = TArrayView<FEventLoadNode>(Node, EEventLoadNode::Package_NumPhases);
    }

	FObjectExport& GetExport(FPackageIndex Index)
	{
		Ncheck(!Index.IsNull() && Index.IsExport());
		return ObjectExports[Index.ToExport()];
	}
	FObjectImport& GetImport(FPackageIndex Index)
	{
        Ncheck(!Index.IsNull() && Index.IsImport());
        return ObjectImports[Index.ToImport()];
	}

    FEventLoadNode& GetPackageNode(EEventLoadNode Phase)
    {
        return PackageNodes[static_cast<int32>(Phase)];
    }

    int32 GetPriority() const { return Desc.Priority; }

    const std::string& GetPackageName() const { return Desc.PackageName; }

    void StartLoading();

    void ImportPackagesRecursive();

    void CreateLinkerLoadExports();

	// static void Event_ProcessExportBundle(FAsyncPackage* Package);
	static void Event_LoadImports(FAsyncPackage* Package);
	static void Event_LoadExports(FAsyncPackage* Package);
	static void Event_ProcessPackageSummary(FAsyncPackage* Package);
	static void Event_DependenciesReady(FAsyncPackage* Package);
	static void Event_ExportsDone(FAsyncPackage* Package);
	static void Event_PostLoadExportBundle(FAsyncPackage* Package);
	static void Event_DeferredPostLoadExportBundle(FAsyncPackage* Package);

    void DependsOn(std::weak_ptr<FAsyncPackage> ImportPackage);

    FAsyncPackageData Data;

	struct FAllDependenciesState
	{
        TArray<std::weak_ptr<FAsyncPackage>> WaitingForPackages;
        TArray<std::weak_ptr<FAsyncPackage>> PackagesWaitingForThis;
	};
    FAllDependenciesState AllDependenciesFullyLoadedState;

    void AddPackagesWaitingForThis(std::weak_ptr<FAsyncPackage> Package)
    {
        AllDependenciesFullyLoadedState.PackagesWaitingForThis.Add(Package);
    }

};

struct FAsyncLoadEventQueue
{
    std::mutex EventQueueMutex;
    TIoPriorityQueue<FEventLoadNode> EventQueue;

    void UpdatePackagePriority(std::shared_ptr<FAsyncPackage> Package);

    void Push(FEventLoadNode* Node)
    {
        std::lock_guard<std::mutex> Lock(EventQueueMutex);
        EventQueue.Push(Node, Node->Priority);
    }

    bool PopAndExecute()
    {
        FEventLoadNode* Node = nullptr;
        {
            std::lock_guard<std::mutex> Lock(EventQueueMutex);
            if (EventQueue.IsEmpty())
            {
                return false;
            }
            Node = EventQueue.Pop();
        }
        Node->Execute();
        return true;
    }
};

class FAsyncLoadingThread : public FRunnable
{
public:
    FAsyncLoadingThread();

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;

    static FAsyncLoadingThread& Get();

    /** From Game Thread. Returns the request id. */
    int32 LoadPackage(const std::string& PackageName, int32 Priority = 0);

    void FlushAsyncLoading(const TArray<int32>& RequestIds);

    std::shared_ptr<FAsyncPackage> FindOrInsertPackage(const FAsyncPackageDesc& Desc);

    void TickAsyncThreadFromGameThread();

    void ProcessRequestedPackages();

    std::weak_ptr<FAsyncPackage> GetPackageByRequestId(uint64 RequestId)
    {
        std::lock_guard<std::mutex> Lock(RequestIdToPackageCritical);
        return RequestIdToPackage[RequestId];
    }

    void ProcessLoadedPackagesFromGameThread();

    std::mutex PackageRequestQueueCritical;
    TArray<FPackageRequest> PackageRequestQueue;

    std::mutex AsyncPackagesCritical;
    TMap<std::string, std::shared_ptr<FAsyncPackage>> AsyncPackageLookup;
    
    std::mutex RequestIdToPackageCritical;
    TMap<uint64, std::weak_ptr<FAsyncPackage>> RequestIdToPackage;

    std::mutex LoadedPackagesToProcessCritical;
    TArray<FAsyncPackage*> LoadedPackagesToProcess;

    std::atomic<uint64> RequestId = 0;

    TArray<FAsyncLoadEventSpec> EventSpecs;
    
    FAsyncLoadEventQueue EventQueue;

private:

    void UpdatePackagePriorityRecursive(std::shared_ptr<FAsyncPackage> Package, int32 NewPriority);

    std::atomic<bool> bShouldExit = false;

};

bool IsInAsyncLoadingThread();

}