#include <fstream>
#include "AsyncLoading.h"
#include "Common/Path.h"
#include "Common/CoreUObject/Class.h"

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
    Spec->Func(Package);
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
        FAsyncPackageDesc Desc{{INDEX_NONE}, GetPriority(), Import.PackageName};
        std::shared_ptr<FAsyncPackage> ImportPackage = AsyncLoadingThread.FindOrInsertPackage(Desc);
        DependsOn(ImportPackage);
    }
}

void FAsyncPackage::DependsOn(std::weak_ptr<FAsyncPackage> WeakImportPackage)
{
    Data.ImportedAsyncPackages.Add(WeakImportPackage);
    AllDependenciesFullyLoadedState.WaitingForPackages.Add(WeakImportPackage);
    if (auto ImportPackage = WeakImportPackage.lock())
    {
        ImportPackage->AddPackagesWaitingForThis(weak_from_this());
        if (ImportPackage->AsyncPackageLoadingState < EAsyncPackageLoadingState::Complete)
        {
            GetPackageNode(Package_DependenciesReady).AddBarrier();
        }
    }
}

void FAsyncPackage::Event_ProcessPackageSummary(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::ProcessPackageSummary;
    std::string MetaFileName = FPackagePath::LongPackageNameToMetaFileName(Package->GetPackageName());
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
    // TODO: use async io
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
    Package->GetPackageNode(Package_LoadImports).ReleaseBarrier();
}

static void RecursiveLoadImports(FObjectImport& Import, FAsyncPackage* Package)
{
    if (Import.XObject != nullptr)
    {
        return;
    }
    FObjectImport& Outer = Package->GetImport(Import.OuterIndex);
    if (Outer.XObject == nullptr)
    {
        RecursiveLoadImports(Outer, Package);
    }
    Import.XObject = FindObject(Outer.XObject, Import.ObjectName);
}

void FAsyncPackage::Event_LoadImports(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::LoadImports;
    for (FObjectImport& Import : Package->ObjectImports)
    {
        RecursiveLoadImports(Import, Package);
    }
    Package->GetPackageNode(Package_LoadExports).ReleaseBarrier();
}

static void RecursiveCreateExports(FObjectExport& Export, FAsyncPackage* Package)
{
    if (Export.Object != nullptr)
    {
        return;
    }
    FObjectExport& Outer = Package->GetExport(Export.OuterIndex);
    if (Outer.Object == nullptr)
    {
        RecursiveCreateExports(Outer, Package);
    }
    FStaticConstructObjectParameters Params;
    NClass* Class = Cast<NClass>(Package->GetImport(Export.ClassIndex).XObject);
    Params.Class = Class;
    Params.Outer = Outer.Object;
    Params.Name = Export.ObjectName;
    Export.Object = StaticConstructObject_Internal(Params);
}

void FAsyncPackage::Event_LoadExports(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::LoadExports;
    for (FObjectExport& Export : Package->ObjectExports)
    {
        RecursiveCreateExports(Export, Package);
    }
    std::string FileName = FPackagePath::LongPackageNameToFileName(Package->GetPackageName());
    nlohmann::json Json;
    std::ifstream(FileName) >> Json;
    for (int Index = 0; Index < Package->ObjectExports.Num(); ++Index)
    {
        FObjectExport& Export = Package->ObjectExports[Index];
        nlohmann::json ObjJson = Json["Objects"][Index];
        FArchive Ar(ObjJson, true);
        Export.Object->Serialize(Ar);
    }
    Package->GetPackageNode(Package_ExportsSerialized).ReleaseBarrier();
}

void FAsyncPackage::Event_ExportsDone(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::ExportsDone;
    Package->GetPackageNode(ExportBundle_DeferredPostLoad).ReleaseBarrier();
}

void FAsyncPackage::Event_DeferredPostLoadExportBundle(FAsyncPackage* Package)
{
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::DeferredPostLoad;
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
	EventSpecs.SetNum(EEventLoadNode::Package_NumPhases + EEventLoadNode::ExportBundle_NumPhases);
	EventSpecs[EEventLoadNode::Package_ProcessSummary] = { &FAsyncPackage::Event_ProcessPackageSummary, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_DependenciesReady] = { &FAsyncPackage::Event_DependenciesReady, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_LoadImports] = { &FAsyncPackage::Event_LoadImports, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_LoadExports] = { &FAsyncPackage::Event_LoadExports, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_ExportsSerialized] = { &FAsyncPackage::Event_ExportsDone, &EventQueue, true };

	// EventSpecs[EEventLoadNode::Package_NumPhases + EEventLoadNode::ExportBundle_Process] = { &FAsyncPackage::Event_ProcessExportBundle, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_NumPhases + EEventLoadNode::ExportBundle_DeferredPostLoad] = { &FAsyncPackage::Event_DeferredPostLoadExportBundle, &EventQueue, false };

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
        EventQueue.PopAndExecute();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
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

std::shared_ptr<FAsyncPackage> FAsyncLoadingThread::FindOrInsertPackage(const FAsyncPackageDesc& Desc)
{
    std::shared_ptr<FAsyncPackage> Package = nullptr;
    {
        std::lock_guard<std::mutex> Lock(AsyncPackagesCritical);
        Package = AsyncPackageLookup.FindRef(Desc.PackageName);
        if (!Package)
        {
            Package = std::make_shared<FAsyncPackage>(
                Desc, 
                *this,
                EventSpecs.GetData());
            AsyncPackageLookup.Add(Desc.PackageName, Package);
            Package->GetPackageNode(Package_ProcessSummary).AddBarrier();
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
    TArray<FAsyncPackage*> LocalLoadedPackagesToProcess;
    {
        std::lock_guard<std::mutex> Lock(LoadedPackagesToProcessCritical);
        std::swap(LocalLoadedPackagesToProcess, LoadedPackagesToProcess);
    }
    for (int32 PackageIndex = 0; PackageIndex < LocalLoadedPackagesToProcess.Num(); ++PackageIndex)
    {
        std::shared_ptr<FAsyncPackage> Package = LocalLoadedPackagesToProcess[PackageIndex]->shared_from_this();
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
}

bool IsInAsyncLoadingThread()
{
    return std::this_thread::get_id() == GAsyncLoadingThreadId;
}

}
