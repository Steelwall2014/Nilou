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
    int32 RequestID;

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
	Package_CreateLinkerLoadExports,
	Package_ResolveLinkerLoadImports,
	Package_PreloadLinkerLoadExports,
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
	CreateLinkerLoadExports,
	WaitingForLinkerLoadDependencies,
	ResolveLinkerLoadImports,
	PreloadLinkerLoadExports,
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
    using FAsyncLoadEventFunc = std::function<void(FAsyncPackage*, int32)>;
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

    FAsyncPackage(const FAsyncPackageDesc& Desc, FAsyncLoadEventSpec* EventSpecs)
        : Desc(Desc)
    {
        FEventLoadNode* Node = reinterpret_cast<FEventLoadNode*>(PackageNodesMemory);
        for (int32 Phase = 0; Phase < EEventLoadNode::Package_NumPhases; ++Phase)
        {
            new (Node + Phase) FEventLoadNode(&EventSpecs[Phase], this, Desc.Priority);
        }
        PackageNodes = TArrayView<FEventLoadNode>(Node, EEventLoadNode::Package_NumPhases);
    }

    FEventLoadNode& GetPackageNode(EEventLoadNode Phase)
    {
        return PackageNodes[static_cast<int32>(Phase)];
    }

    int32 GetPriority() const { return Desc.Priority; }

    int32 GetRequestId() const { return Desc.RequestID; }

    const std::string& GetPackageName() const { return Desc.PackageName; }

    void CreateLinkerLoadExports();

	static void Event_ProcessExportBundle(FAsyncPackage* Package, int32 ExportBundleIndex);
	static void Event_CreateLinkerLoadExports(FAsyncPackage* Package, int32);
	static void Event_ResolveLinkerLoadImports(FAsyncPackage* Package, int32);
	static void Event_PreloadLinkerLoadExports(FAsyncPackage* Package, int32);
	static void Event_ProcessPackageSummary(FAsyncPackage* Package, int32);
	static void Event_DependenciesReady(FAsyncPackage* Package, int32);
	static void Event_ExportsDone(FAsyncPackage* Package, int32);
	static void Event_PostLoadExportBundle(FAsyncPackage* Package, int32 ExportBundleIndex);
	static void Event_DeferredPostLoadExportBundle(FAsyncPackage* Package, int32 ExportBundleIndex);

    void DependsOn(std::weak_ptr<FAsyncPackage> ImportPackage);

    FAsyncPackageData Data;

};

struct FAsyncLoadEventQueue
{
    std::mutex EventQueueMutex;
    TIoPriorityQueue<FEventLoadNode> EventQueue;

    void UpdatePackagePriority(FAsyncPackage* Package);

    void Push(FEventLoadNode* Node)
    {
        std::lock_guard<std::mutex> Lock(EventQueueMutex);
        EventQueue.Push(Node, Node->Priority);
    }
};

class FAsyncLoadingThread : public FRunnable
{
public:
    FAsyncLoadingThread();

    static FAsyncLoadingThread& Get();

    /** From Game Thread. Returns the request id. */
    int32 LoadPackage(const std::string& PackageName, int32 Priority = 0);

    FAsyncPackage* FindOrInsertPackage(const FAsyncPackageDesc& Desc);

    void TickAsyncThreadFromGameThread();

    void ProcessRequestedPackages();

    std::weak_ptr<FAsyncPackage> GetPackageByRequestId(uint64 RequestId)
    {
        std::lock_guard<std::mutex> Lock(RequestIdToPackageMutex);
        return RequestIdToPackage[RequestId];
    }

    std::mutex PackageRequestQueueMutex;
    TArray<FPackageRequest> PackageRequestQueue;

    std::mutex LoadingPackagesMutex;
    TArray<std::shared_ptr<FAsyncPackage>> LoadingPackages;
    
    std::mutex RequestIdToPackageMutex;
    TMap<uint64, std::weak_ptr<FAsyncPackage>> RequestIdToPackage;

    std::atomic<uint64> RequestId = 0;

    TArray<FAsyncLoadEventSpec> EventSpecs;
    
    FAsyncLoadEventQueue EventQueue;

    std::thread::id ThreadId;

};

bool IsInAsyncLoadingThread();

}