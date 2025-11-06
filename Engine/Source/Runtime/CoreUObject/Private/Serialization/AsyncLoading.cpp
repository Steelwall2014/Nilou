#include <fstream>
#include <stack>
#include "Serialization/AsyncLoading.h"
#include "Misc/Paths.h"
#include "NObject/Class.h"
#include "HAL/PlatformMisc.h"

namespace nilou {

namespace fs = std::filesystem;

std::thread::id GAsyncLoadingThreadId;
FAsyncLoadingThread* GAsyncLoadingThread = nullptr;

void FEventLoadNode::Fire()
{
    if (Spec->bExecuteImmediately)
    {
        Execute();
    }
    else
    {
        Spec->EventQueue->Push(this);
    }
}

void FEventLoadNode::Execute()
{
    NILOU_LOG(Display, "{}, {}, Execute, Priority: {}", Package->GetPackageName(), Spec->Name, Priority);
    Spec->Func(Package);
}

void FEventLoadNode::ReleaseBarrier()
{
    Ncheck(BarrierCount > 0);
    if (--BarrierCount == 0)
    {
        Fire();
    }
}

void FEventLoadNode::AddBarrier()
{
    ++BarrierCount;
}

FAsyncPackage::FAsyncPackage(const FAsyncPackageDesc& InDesc, FAsyncLoadingThread& InAsyncLoadingThread, FAsyncLoadEventSpec* EventSpecs)
    : Desc(InDesc)
    , AsyncLoadingThread(InAsyncLoadingThread)
{
    for (int32 Phase = 0; Phase < EEventLoadNode::Package_NumPhases; ++Phase)
    {
        new (&PackageNodes[Phase]) FEventLoadNode(&EventSpecs[Phase], this, Desc.Priority);
    }
}

void FAsyncPackage::StartLoading()
{
	AsyncPackageLoadingState = EAsyncPackageLoadingState::WaitingForIo;
    GetPackageNode(Package_ProcessSummary).ReleaseBarrier();
}

void FAsyncPackage::ImportPackagesRecursive()
{
    for (FObjectImport& Import : ObjectImports)
    {
        std::shared_ptr<FAsyncPackage> ImportPackage = AsyncLoadingThread.FindPackage(Import.PackageName);
        if (!ImportPackage)
        {
            if (NPackage* PackageObject = FindPackage(Import.PackageName))
            {
                continue;
            }
            else 
            {
                FAsyncPackageDesc Desc{{INDEX_NONE}, GetPriority(), Import.PackageName};
                ImportPackage = AsyncLoadingThread.FindOrInsertPackage(Desc);
            }
        }
        DependsOn(ImportPackage);
    }
}

void FAsyncPackage::DependsOn(std::shared_ptr<FAsyncPackage> ImportPackage)
{
    if (!Data.ImportedAsyncPackages.Contains(ImportPackage))
    {
        Data.ImportedAsyncPackages.Add(ImportPackage);
        if (ImportPackage->AsyncPackageLoadingState < EAsyncPackageLoadingState::DependenciesReady)
        {
            GetPackageNode(Package_CreateLinkerLoadExports).AddBarrier();
            ImportPackage->OnPackageReachState[int32(EAsyncPackageLoadingState::DependenciesReady)].Add(
                [This=shared_from_this()](FAsyncPackage* Dependency) 
                {
                    This->GetPackageNode(Package_CreateLinkerLoadExports).ReleaseBarrier();
                });
        }

        if (ImportPackage->AsyncPackageLoadingState < EAsyncPackageLoadingState::CreateLinkerLoadExports)
        {
            GetPackageNode(Package_ResolveLinkerLoadImports).AddBarrier();
            ImportPackage->OnPackageReachState[int32(EAsyncPackageLoadingState::CreateLinkerLoadExports)].Add(
                [This=shared_from_this()](FAsyncPackage* Dependency) 
                { 
                    This->GetPackageNode(Package_ResolveLinkerLoadImports).ReleaseBarrier();
                });
        }
    }
}

void FAsyncPackage::Event_ProcessPackageSummary(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::ProcessPackageSummary;
    std::string MetaFileName = FPackageName::LongPackageNameToMetaFileName(Package->GetPackageName());
    if (!fs::exists(MetaFileName))
    {
        Package->LoadResult = EAsyncLoadingResult::FailedMissing;
        Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::Finalize;
        std::lock_guard<std::mutex> Lock(Package->AsyncLoadingThread.LoadedPackagesToProcessCritical);
        Package->AsyncLoadingThread.LoadedPackagesToProcess.Add(Package);
        return;
    }
    NPackage* LinkerRoot = NewObject<NPackage>(nullptr, Package->GetPackageName());
    Package->LinkerRoot = LinkerRoot;
    nlohmann::json Json;
    std::ifstream(MetaFileName) >> Json;
    {
        FArchive Ar(Json["ObjectImports"], true);
        Serialize(Ar, Package->ObjectImports);
    }
    {
        FArchive Ar(Json["ObjectExports"], true);
        Serialize(Ar, Package->ObjectExports);
    }
    Package->ImportPackagesRecursive();
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::WaitingForDependencies;
    Package->GetPackageNode(Package_DependenciesReady).ReleaseBarrier();
}

void FAsyncPackage::Event_DependenciesReady(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::DependenciesReady;
    for (auto& Callback : Package->OnPackageReachState[int32(EAsyncPackageLoadingState::DependenciesReady)])
    {
        Callback(Package);
    }
    Package->GetPackageNode(Package_CreateLinkerLoadExports).ReleaseBarrier();
}

static void RecursiveLoadImports(FObjectImport& Import, FAsyncPackage* Package)
{
    if (Import.XObject != nullptr)
    {
        return;
    }
    NObject* OuterObject = nullptr;
    if (!Import.OuterIndex.IsNull())
    {
        FObjectImport& Outer = Package->GetImport(Import.OuterIndex);
        if (Outer.XObject == nullptr)
        {
            RecursiveLoadImports(Outer, Package);
        }
        OuterObject = Outer.XObject;
    }
    Import.XObject = FindObject(OuterObject, Import.ObjectName);
    Ncheck(Import.XObject);
}

static void RecursiveCreateExports(FObjectExport& Export, FAsyncPackage* Package)
{
    if (Export.Object != nullptr)
    {
        return;
    }
    NObject* OuterObject = nullptr;
    if (Export.OuterIndex.IsNull())
    {
        OuterObject = Package->LinkerRoot;
    }
    else
    {
        FObjectExport& Outer = Package->GetExport(Export.OuterIndex);
        if (Outer.Object == nullptr)
        {
            RecursiveCreateExports(Outer, Package);
        }
        OuterObject = Outer.Object;
    }
    FObjectImport& ClassImport = Package->GetImport(Export.ClassIndex);
    if (!ClassImport.XObject)
    {
        RecursiveLoadImports(ClassImport, Package);
    }
    FStaticConstructObjectParameters Params;
    NClass* Class = Cast<NClass>(ClassImport.XObject);
    Params.Class = Class;
    Params.Outer = OuterObject;
    Params.Name = Export.ObjectName;
    Params.Flags = EObjectFlags::NeedLoad | EObjectFlags::NeedPostLoad;
    Export.Object = StaticConstructObject_Internal(Params);
}

void FAsyncPackage::Event_CreateLinkerLoadExports(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::CreateLinkerLoadExports;
    for (auto& Callback : Package->OnPackageReachState[int32(EAsyncPackageLoadingState::CreateLinkerLoadExports)])
    {
        Callback(Package);
    }
    for (FObjectExport& Export : Package->ObjectExports)
    {
        RecursiveCreateExports(Export, Package);
    }
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::WaitingForLinkerLoadDependencies;
    Package->GetPackageNode(Package_ResolveLinkerLoadImports).ReleaseBarrier();
}

void FAsyncPackage::Event_ResolveLinkerLoadImports(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::ResolveLinkerLoadImports;
    for (FObjectImport& Import : Package->ObjectImports)
    {
        RecursiveLoadImports(Import, Package);
    }
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::PreloadLinkerLoadExports;
    Package->GetPackageNode(Package_PreloadLinkerLoadExports).ReleaseBarrier();
}

void FAsyncPackage::Event_PreloadLinkerLoadExports(FAsyncPackage* Package)
{
    std::string FileName = FPackageName::LongPackageNameToFileName(Package->GetPackageName());
    nlohmann::json Json;
    std::ifstream(FileName) >> Json;
    for (int Index = 0; Index < Package->ObjectExports.Num(); ++Index)
    {
        FObjectExport& Export = Package->ObjectExports[Index];
        nlohmann::json ObjJson = Json["Objects"][Index];
        FArchive Ar(ObjJson, true);
        Export.Object->Serialize(Ar);
    }
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::ExportsDone;
    Package->GetPackageNode(Package_ExportsSerialized).ReleaseBarrier();
}

void FAsyncPackage::Event_ExportsDone(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::DeferredPostLoad;
    Package->GetPackageNode(Package_DeferredPostLoad).ReleaseBarrier();
}

void FAsyncPackage::Event_DeferredPostLoadExportBundle(FAsyncPackage* Package)
{
    for (int Index = 0; Index < Package->ObjectExports.Num(); ++Index)
    {
        FObjectExport& Export = Package->ObjectExports[Index];
        Export.Object->ConditionalPostLoad();
    }
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::Finalize;
    std::lock_guard<std::mutex> Lock(Package->AsyncLoadingThread.LoadedPackagesToProcessCritical);
    Package->AsyncLoadingThread.LoadedPackagesToProcess.Add(Package);
}

void FAsyncLoadEventQueue::UpdatePackagePriority(std::shared_ptr<FAsyncPackage> Package)
{
	std::lock_guard<std::mutex> Lock(EventQueueMutex);
	for (FEventLoadNode& Node : Package->PackageNodes)
	{
        if (Node.Spec->EventQueue == this && Node.Priority < Node.Package->GetPriority())
        {
            EventQueue.Reprioritize(&Node, Node.Package->GetPriority());
        }
	}
}

FAsyncLoadingThread::FAsyncLoadingThread()
{
	EventSpecs.SetNum(EEventLoadNode::Package_NumPhases);
	EventSpecs[EEventLoadNode::Package_ProcessSummary] = { &FAsyncPackage::Event_ProcessPackageSummary, &EventQueue, true, "ProcessPackageSummary" };
	EventSpecs[EEventLoadNode::Package_DependenciesReady] = { &FAsyncPackage::Event_DependenciesReady, &EventQueue, false, "DependenciesReady" };
	EventSpecs[EEventLoadNode::Package_ResolveLinkerLoadImports] = { &FAsyncPackage::Event_ResolveLinkerLoadImports, &EventQueue, false, "ResolveLinkerLoadImports" };
	EventSpecs[EEventLoadNode::Package_CreateLinkerLoadExports] = { &FAsyncPackage::Event_CreateLinkerLoadExports, &EventQueue, false, "CreateLinkerLoadExports" };
	EventSpecs[EEventLoadNode::Package_PreloadLinkerLoadExports] = { &FAsyncPackage::Event_PreloadLinkerLoadExports, &EventQueue, false, "PreloadLinkerLoadExports" };
	EventSpecs[EEventLoadNode::Package_ExportsSerialized] = { &FAsyncPackage::Event_ExportsDone, &EventQueue, true, "ExportsSerialized" };
	// EventSpecs[EEventLoadNode::Package_PostLoad] = { &FAsyncPackage::Event_PostLoadExportBundle, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_DeferredPostLoad] = { &FAsyncPackage::Event_DeferredPostLoadExportBundle, &MainThreadEventQueue, false, "DeferredPostLoad" };

}

bool FAsyncLoadingThread::Init()
{
    GAsyncLoadingThreadId = std::this_thread::get_id();
    GAsyncLoadingThread = this;
    return true;
}

uint32 FAsyncLoadingThread::Run()
{
    while (!bShouldExit)
    {
        bool bDidSomething = EventQueue.PopAndExecute();
        if (!bDidSomething)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }
    return 0;
}

void FAsyncLoadingThread::Stop()
{
    bShouldExit = true;
}

FAsyncLoadingThread& FAsyncLoadingThread::Get()
{
    return *GAsyncLoadingThread;
}

int32 FAsyncLoadingThread::LoadPackage(const std::string& PackageName, int32 Priority)
{
    Ncheck(!IsInAsyncLoadingThread());
    int32 RequestId = this->RequestId.fetch_add(1);
    FAsyncPackageDesc Desc;
    Desc.RequestIds = { RequestId };
    Desc.Priority = Priority;
    Desc.PackageName = PackageName;
    std::shared_ptr<FAsyncPackage> Package = FindOrInsertPackage(Desc);
    return RequestId;
}

void FAsyncLoadingThread::FlushAsyncLoading(const TArray<int32>& RequestIds)
{
    while (true)
    {
        ProcessLoadedPackagesFromGameThread();
        bool bAllRequestsDone = true;
        for (int32 RequestId : RequestIds)
        {
            std::lock_guard<std::mutex> Lock(RequestIdToPackageCritical);
            if (RequestIdToPackage.Contains(RequestId))
            {
                bAllRequestsDone = false;
                break;
            }
        }
        if (bAllRequestsDone)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::shared_ptr<FAsyncPackage> FAsyncLoadingThread::FindPackage(const std::string& PackageName)
{
    std::lock_guard<std::mutex> Lock(AsyncPackagesCritical);
    return AsyncPackageLookup.FindRef(PackageName);
}

std::shared_ptr<FAsyncPackage> FAsyncLoadingThread::FindOrInsertPackage(const FAsyncPackageDesc& Desc)
{
    std::shared_ptr<FAsyncPackage> Package = nullptr;
    {
        bool bInserted = false;
        {
            std::lock_guard<std::mutex> Lock(AsyncPackagesCritical);
            Package = AsyncPackageLookup.FindRef(Desc.PackageName);
            if (!Package)
            {
                Package = std::shared_ptr<FAsyncPackage>(new FAsyncPackage(
                    Desc, 
                    *this,
                    EventSpecs.GetData()));
                AsyncPackageLookup.Add(Desc.PackageName, Package);
                bInserted = true;
            }
        }
        if (bInserted)
        {
            Package->StartLoading();
        }
    }
    for (int32 RequestId : Desc.RequestIds)
    {
        Package->Desc.RequestIds.AddUnique(RequestId);
        RequestIdToPackage.Add(RequestId, Package);
    }
    UpdatePackagePriorityRecursive(Package, Desc.Priority);
    return Package;
}

void FAsyncLoadingThread::ProcessLoadedPackagesFromGameThread()
{
    MainThreadEventQueue.PopAndExecute();
    TArray<FAsyncPackage*> LocalLoadedPackagesToProcess;
    {
        std::lock_guard<std::mutex> Lock(LoadedPackagesToProcessCritical);
        std::swap(LocalLoadedPackagesToProcess, LoadedPackagesToProcess);
    }
    for (int32 PackageIndex = 0; PackageIndex < LocalLoadedPackagesToProcess.Num(); ++PackageIndex)
    {
        std::shared_ptr<FAsyncPackage> Package = LocalLoadedPackagesToProcess[PackageIndex]->shared_from_this();
        Ncheck(Package->LoadResult == EAsyncLoadingResult::Succeeded);
        {
            std::lock_guard<std::mutex> Lock(AsyncPackagesCritical);
            AsyncPackageLookup.Remove(Package->Desc.PackageName);
        }
        {
            std::lock_guard<std::mutex> Lock(RequestIdToPackageCritical);
            for (int32 RequestId : Package->Desc.RequestIds)
            {
                RequestIdToPackage.Remove(RequestId);
            }
        }
        for (FObjectExport& Export : Package->ObjectExports)
        {
            Export.Object->ClearFlags(EObjectFlags::NeedLoad);
            Export.Object->SetFlags(EObjectFlags::WasLoaded);
        }
        Package->LinkerRoot->SetFlags(EObjectFlags::WasLoaded);
    }
}

void FAsyncLoadingThread::UpdatePackagePriorityRecursive(std::shared_ptr<FAsyncPackage> Package, int32 NewPriority)
{
	if (Package->Desc.Priority >= NewPriority)
	{
		return;
	}
	Package->Desc.Priority = NewPriority;
	for (std::weak_ptr<FAsyncPackage> WeakImportedPackage : Package->Data.ImportedAsyncPackages)
	{
		if (auto ImportedPackage = WeakImportedPackage.lock())
		{
			UpdatePackagePriorityRecursive(ImportedPackage, NewPriority);
		}
	}
	EventQueue.UpdatePackagePriority(Package);
    MainThreadEventQueue.UpdatePackagePriority(Package);
}

bool IsInAsyncLoadingThread()
{
    return std::this_thread::get_id() == GAsyncLoadingThreadId;
}

}
