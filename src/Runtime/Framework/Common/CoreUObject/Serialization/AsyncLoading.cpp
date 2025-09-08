#include "AsyncLoading.h"

namespace nilou {

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
    Spec->Func(Package, 0);
}

std::string LongPackageNameToFileName(const std::string& PackageName)
{
    return (FPath::ContentDir() / std::filesystem::path(PackageName + ".nasset")).generic_string();
}

void FAsyncPackage::CreateLinkerLoadExports()
{
    for (auto& Export : LinkerRoot->ObjectExports)
    {
    }
}

void FAsyncPackage::Event_CreateLinkerLoadExports(FAsyncPackage* Package, int32)
{
    Package->CreateLinkerLoadExports();
}

void FAsyncPackage::Event_ProcessPackageSummary(FAsyncPackage* Package, int32)
{
    NPackage* LinkerRoot = NewObject<NPackage>(Package->GetPackageName());
    Package->LinkerRoot = LinkerRoot;
    std::string FileName = LongPackageNameToFileName(Package->GetPackageName());
    nlohmann::json Json;
    std::ifstream in(FileName);
    in >> Json;
    FArchive Ar(Json, true);
    LinkerRoot->Serialize(Ar);
    for (auto& Import : LinkerRoot->ObjectImports)
    {
        int32 RequestId = FAsyncLoadingThread::Get().LoadPackage(Import.PackageName, Package->GetPriority());
        std::weak_ptr<FAsyncPackage> ImportPackage = FAsyncLoadingThread::Get().GetPackageByRequestId(RequestId);
        Package->DependsOn(ImportPackage);
    }
    Package->GetPackageNode(Package_DependenciesReady).ReleaseBarrier();
}

void FAsyncPackage::DependsOn(std::weak_ptr<FAsyncPackage> ImportPackage)
{
    Data.ImportedAsyncPackages.Add(ImportPackage);
}

void FAsyncPackage::Event_DependenciesReady(FAsyncPackage* Package, int32)
{
    for (std::weak_ptr<FAsyncPackage> WeakImport : Package->Data.ImportedAsyncPackages)
    {
        if (auto ImportPackage = WeakImport.lock())
        {
            if (ImportPackage->AsyncPackageLoadingState < EAsyncPackageLoadingState::DependenciesReady)
            {
                Package->GetPackageNode(Package_CreateLinkerLoadExports).AddBarrier();
            }
        }
    }
    Package->AsyncPackageLoadingState = EAsyncPackageLoadingState::DependenciesReady;
    Package->GetPackageNode(Package_CreateLinkerLoadExports).ReleaseBarrier();
}

void FAsyncPackage::Event_ExportsDone(FAsyncPackage* Package, int32)
{
    
}

void FAsyncLoadEventQueue::UpdatePackagePriority(FAsyncPackage* Package)
{
	std::lock_guard<std::mutex> Lock(EventQueueMutex);
	auto ReprioritizeNode = [this](FEventLoadNode& Node)
	{
        if (Node.Spec->EventQueue == this && Node.Priority < Node.Package->GetPriority())
        {
            EventQueue.Reprioritize(&Node, Node.Package->GetPriority());
        }
	};

	for (FEventLoadNode& Node : Package->PackageNodes)
	{
		ReprioritizeNode(Node);
	}
}

FAsyncLoadingThread::FAsyncLoadingThread()
{
	EventSpecs.SetNum(EEventLoadNode::Package_NumPhases + EEventLoadNode::ExportBundle_NumPhases);
	EventSpecs[EEventLoadNode::Package_ProcessSummary] = { &FAsyncPackage::Event_ProcessPackageSummary, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_DependenciesReady] = { &FAsyncPackage::Event_DependenciesReady, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_CreateLinkerLoadExports] = { &FAsyncPackage::Event_CreateLinkerLoadExports, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_ResolveLinkerLoadImports] = { &FAsyncPackage::Event_ResolveLinkerLoadImports, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_PreloadLinkerLoadExports] = { &FAsyncPackage::Event_PreloadLinkerLoadExports, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_ExportsSerialized] = { &FAsyncPackage::Event_ExportsDone, &EventQueue, true };

	EventSpecs[EEventLoadNode::Package_NumPhases + EEventLoadNode::ExportBundle_Process] = { &FAsyncPackage::Event_ProcessExportBundle, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_NumPhases + EEventLoadNode::ExportBundle_PostLoad] = { &FAsyncPackage::Event_PostLoadExportBundle, &EventQueue, false };
	EventSpecs[EEventLoadNode::Package_NumPhases + EEventLoadNode::ExportBundle_DeferredPostLoad] = { &FAsyncPackage::Event_DeferredPostLoadExportBundle, &EventQueue, false };

}

int32 FAsyncLoadingThread::LoadPackage(const std::string& PackageName, int32 Priority)
{
    {
        std::lock_guard<std::mutex> Lock(LoadingPackagesMutex);
        for (auto& Package : LoadingPackages)
        {
            if (Package->Desc.PackageName == PackageName)
            {
                Package->Desc.Priority = Priority;
                EventQueue.UpdatePackagePriority(Package.get());
                return Package->Desc.RequestID;
            }
        }
    }
    {
        std::lock_guard<std::mutex> Lock(PackageRequestQueueMutex);
        for (FPackageRequest& PackageRequest : PackageRequestQueue)
        {
            if (PackageRequest.PackageName == PackageName)
            {
                return PackageRequest.RequestId;
            }
        }
        int32 RequestId = this->RequestId.fetch_add(1);
        PackageRequestQueue.Add(FPackageRequest(PackageName, Priority, RequestId));
    }
    if (IsInAsyncLoadingThread())
    {
        ProcessRequestedPackages();
    }
    return RequestId;
}

FAsyncPackage* FAsyncLoadingThread::FindOrInsertPackage(const FAsyncPackageDesc& Desc)
{

}

void FAsyncLoadingThread::TickAsyncThreadFromGameThread()
{

}

void FAsyncLoadingThread::ProcessRequestedPackages()
{
    TArray<std::shared_ptr<FAsyncPackage>> NewPackages;
    std::lock_guard<std::mutex> Lock(PackageRequestQueueMutex);
    for (FPackageRequest& PackageRequest : PackageRequestQueue)
    {
        auto Package = std::make_shared<FAsyncPackage>(
            PackageRequest.PackageName, 
            PackageRequest.RequestId, 
            PackageRequest.Priority, 
            &EventSpecs[0]);
        RequestIdToPackage.Add(PackageRequest.RequestId, Package->weak_from_this());
        NewPackages.Add(Package);
    }
    PackageRequestQueue.Empty();
    {
        std::lock_guard<std::mutex> Lock(LoadingPackagesMutex);
        LoadingPackages.Append(NewPackages);
    }
    for (auto& Package : NewPackages)
    {

    }
}

bool IsInAsyncLoadingThread()
{
    return std::this_thread::get_id() == FAsyncLoadingThread::Get().ThreadId;
}

}
