#include <fstream>
#include <sstream>
#include <regex>

#include "Common/BaseApplication.h"
#include "Common/StaticMeshResources.h"
#include "Common/ContentManager.h"
#include "Material.h"
#include "Common/World.h"
#include "Cesium3DTilesetComponent.h"

#include "Common/PrimitiveUtils.h"

namespace nilou {

    const glm::dmat4 X_UP_TO_Z_UP = glm::dmat4( 0.0, 0.0, 1.0, 0.0,
                                                0.0, 1.0, 0.0, 0.0,
                                                -1.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0);

    const glm::dmat4 Y_UP_TO_Z_UP = glm::dmat4( 1.0, 0.0, 0.0, 0.0,
                                                0.0, 0.0, 1.0, 0.0,
                                                0.0, -1.0, 0.0, 0.0,
                                                0.0, 0.0, 0.0, 1.0);

    using namespace Cesium3DTilesetSelection;

    void BoundingVolumeConvert(FOrientedBoundingBox &Box, tiny3dtiles::BoundingVolume &BoundingVolume, const glm::dmat4 &transform)
    {
        auto &box = BoundingVolume.box;
        dvec3 center = transform * dvec4(box[0], box[1], box[2], 1.0);
        dmat3 halfAxis = dmat3(transform) * dmat3(box[3], box[4], box[5], box[6], box[7], box[8], box[9], box[10], box[11]);
        Box = FOrientedBoundingBox(center, halfAxis);
    }

    std::shared_ptr<Cesium3DTile> Cesium3DTile::BuildTile(
        std::shared_ptr<tiny3dtiles::Tile> Tile, const glm::dmat4 &parentTransform, ETileGltfUpAxis TileGltfUpAxis)
    {
        std::shared_ptr<Cesium3DTile> InternalTile = std::make_shared<Cesium3DTile>();
        InternalTile->TileGltfUpAxis = TileGltfUpAxis;
        InternalTile->GeometricError = Tile->geometricError;
        InternalTile->Refine = Tile->refine;
        InternalTile->Transform = parentTransform * Tile->transform;
        BoundingVolumeConvert(InternalTile->BoundingVolume, Tile->boundingVolume, InternalTile->Transform);
        InternalTile->Content.URI = Tile->content.uri;
        InternalTile->TransformRHI = CreateUniformBuffer<FPrimitiveShaderParameters>();
        if (!Tile->content.b3dm.featureTableJSON.empty())
        {
            nlohmann::json json;
            std::stringstream stream(Tile->content.b3dm.featureTableJSON);
            stream >> json;
            if (json.is_object() && json.contains("RTC_CENTER"))
            {
                nlohmann::json rtc_center = json["RTC_CENTER"];
                if (rtc_center.is_array() && 
                    rtc_center.size() == 3)
                {
                    InternalTile->RtcCenter.x = rtc_center.at(0).get<double>();
                    InternalTile->RtcCenter.y = rtc_center.at(1).get<double>();
                    InternalTile->RtcCenter.z = rtc_center.at(2).get<double>();
                }
            }
        }
        

        if (Tile->content.external_tileset != nullptr)
        {
            std::shared_ptr<Cesium3DTile> ExternalRoot = Cesium3DTileset::Build(Tile->content.external_tileset, InternalTile->Transform)->Root;
            InternalTile->Children.push_back(ExternalRoot);
        }
        else 
        {
            InternalTile->Content.B3dm = Tile->content.b3dm;
        }

        for (auto &child : Tile->children)
        {
            InternalTile->Children.push_back(BuildTile(child, InternalTile->Transform, TileGltfUpAxis));
        }

        return InternalTile;
    }

    std::shared_ptr<Cesium3DTileset> Cesium3DTileset::Build(
        std::shared_ptr<tiny3dtiles::Tileset> Tileset, const glm::dmat4 &parentTransform)
    {
        ETileGltfUpAxis TileGltfUpAxis = ETileGltfUpAxis::Y;
        if (Tileset->asset.find("gltfUpAxis") != Tileset->asset.end())
        {
            if (Tileset->asset["gltfUpAxis"] == "X")
                TileGltfUpAxis = ETileGltfUpAxis::X;
            else if (Tileset->asset["gltfUpAxis"] == "Y")
                TileGltfUpAxis = ETileGltfUpAxis::Y;
            else if (Tileset->asset["gltfUpAxis"] == "Z")
                TileGltfUpAxis = ETileGltfUpAxis::Z;
        }
        std::shared_ptr<Cesium3DTileset> InternalTileset = std::make_shared<Cesium3DTileset>();
        InternalTileset->Asset = Tileset->asset;
        InternalTileset->GeometricError = Tileset->geometricError;
        InternalTileset->Root = Cesium3DTile::BuildTile(Tileset->root, parentTransform, TileGltfUpAxis);
        return InternalTileset;
    }

    // Cesium3DTilesetSelector::Cesium3DTilesetSelector(UCesium3DTilesetComponent *InComponent)
    //     : Component(InComponent)
    // {
    //     TilesToRenderThisFrame = &InComponent->TilesToRenderThisFrame;
    // }

    // void UCesium3DTilesetComponent::DispatchMainThreadTask()
    // {
    //     int TaskCount = MainThreadTaskQueue.size();
    //     for (int TaskIndex = 0; TaskIndex < TaskCount; TaskIndex++)
    //     {
    //         std::unique_lock<std::mutex> lock(mutex);
    //         TileMainThreadTask Task = MainThreadTaskQueue.front();
    //         MainThreadTaskQueue.pop();
    //         lock.unlock();
    //         Task.Func(Task.Result);
    //     }
    // }

    void UCesium3DTilesetComponent::Update(Cesium3DTileset *Tileset, const std::vector<ViewState> &ViewStates)
    {
        TilesToRenderThisFrame.clear();

        // DispatchMainThreadTask();

        UpdateInternal(Tileset->Root.get(), ViewStates);

        UnloadTiles();

        LoadTiles();
    }

    void UCesium3DTilesetComponent::LoadContent(Cesium3DTile *Tile)
    {
        auto LoadFunc = [this, Tile]() {
                    if (!Tile->Content.B3dm.header_loaded())
                        return;
                    std::unique_lock<std::mutex> lock(Tile->mutex, std::try_to_lock);
                    if (!lock.owns_lock())
                        return;
                    tinygltf::Model model;
                    Tile->LoadingState = ETileLoadingState::Loading;
                    Tile->Content.B3dm.load_glb(Tile->Content.URI);
                    tinygltf::TinyGLTF Loader;
                    std::string err;
                    std::string warn;
                    Loader.LoadBinaryFromMemory(&model, &err, &warn, Tile->Content.B3dm.glb.data(), Tile->Content.B3dm.glb.size());
                    auto Result = GameStatics::ParseToStaticMeshes(model, false);
                    std::atomic<bool> RHIInitialized = false;
                    std::mutex rhi_mutex;
                    std::unique_lock<std::mutex> rhi_lock(rhi_mutex);
                    std::condition_variable cv;
                    ENQUEUE_RENDER_COMMAND(LoadContent)(
                        [&Result, this, &RHIInitialized, &cv](FDynamicRHI*) 
                        {
                            Result.InitResource();
                            RHIInitialized = true;
                            cv.notify_all();
                        });
                    if (!RHIInitialized)
                    {
                        cv.wait(rhi_lock, [&RHIInitialized](){ return RHIInitialized.load(); });
                    }
                    Tile->Content.Gltf = Result;
                    Tile->LoadingState = ETileLoadingState::Loaded;
                };
        pool.push_task(LoadFunc);
    }

    void UCesium3DTilesetComponent::LoadTiles()
    {
        for (Cesium3DTile *Tile : LoadQueue)
        {
            LoadContent(Tile);
        }
        LoadQueue.clear();
    }

    void UCesium3DTilesetComponent::UnloadTiles()
    {
        for (Cesium3DTile *Tile : UnloadQueue)
        {
            ENQUEUE_RENDER_COMMAND(UCesium3DTilesetComponent_UnloadTiles)(
                [this, Tile](FDynamicRHI*) 
                {
                    std::unique_lock<std::mutex> lock(Tile->mutex, std::try_to_lock);
                    if (!lock.owns_lock())
                        return;
                    Tile->LoadingState = ETileLoadingState::Unloading;
                    Tile->Content.B3dm.reset_glb();
                    Tile->Content.Gltf.Materials.clear();
                    Tile->Content.Gltf.Textures.clear();
                    Tile->Content.Gltf.StaticMeshes.clear();
                    Tile->Content.Gltf.UniformBuffer = nullptr;
                    Tile->LoadingState = ETileLoadingState::Unloaded;
                });
        }
        UnloadQueue.clear();
    }

    void UCesium3DTilesetComponent::UpdateInternal(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        bool bTileCulled = bEnableFrustumCulling && FrustumCull(Tile, ViewStates);

        if (!bTileCulled)
        {
            bool wantToRefine = !MeetsSSE(Tile, ViewStates);

            if (wantToRefine)
            {
                if (Tile->IsLeaf())
                {
                    AddTileToRenderQueue(Tile);
                }
                else 
                {
                    AddTileToUnloadQueue(Tile);
                    for (std::shared_ptr<Cesium3DTile> tile : Tile->Children)
                    {
                        UpdateInternal(tile.get(), ViewStates);
                    }
                }
            }
            else 
            {
                if (!Tile->Content.B3dm.header_loaded())
                {
                    AddTileToUnloadQueue(Tile);
                    for (std::shared_ptr<Cesium3DTile> tile : Tile->Children)
                    {
                        UpdateInternal(tile.get(), ViewStates);
                    }
                }
                else 
                {
                    AddTileToRenderQueue(Tile);
                }
            }

        }
        else 
        {
            AddTileToUnloadQueue(Tile);
        }

    }

    // void LRUCache::Load(Cesium3DTile *Tile)
    // {
    //     if (Tile->LoadingState == ETileLoadingState::Loaded)
    //     {
    //         LoadedTiles.splice(LoadedTiles.begin(), LoadedTiles, Tile->iter);
    //         return;
    //     }

    //     if (LoadedTiles.size() >= Capacity)
    //     {
    //         auto iter = LoadedTiles.end();
    //         iter--;

    //         Cesium3DTile *LRUTile = *iter;
    //         LRUTile->LoadingState = ETileLoadingState::Unloading;
    //         LRUTile->Content.B3dm = nullptr;
    //         LoadedTiles.erase(iter);
    //         LRUTile->LoadingState = ETileLoadingState::Unloaded;
    //     }

    //     Tile->LoadingState = ETileLoadingState::Loading;
    //     Tile->Content.B3dm = std::make_shared<tiny3dtiles::B3DM>();
    //     std::ifstream stream(Tile->Content.URI, std::ios::binary);
    //     stream >> *Tile->Content.B3dm;
    //     Tile->LoadingState = ETileLoadingState::Loaded;
    //     Tile->iter = LoadedTiles.insert(LoadedTiles.begin(), Tile);
    // }

    void UCesium3DTilesetComponent::AddTileToUnloadQueue(Cesium3DTile *Tile)
    {
        if (Tile->LoadingState == ETileLoadingState::Loaded)
            UnloadQueue.push_back(Tile);
    }

    void UCesium3DTilesetComponent::AddTileToRenderQueue(Cesium3DTile *Tile)
    {
        if (Tile->LoadingState == ETileLoadingState::Unloaded)
            LoadQueue.push_back(Tile);
        TilesToRenderThisFrame.push_back(Tile);
    }

    bool UCesium3DTilesetComponent::FrustumCull(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        bool bCullWithChildrenBounds = !Tile->Children.empty();
        if (bCullWithChildrenBounds)
        {
            if (std::any_of(ViewStates.begin(), ViewStates.end(), 
                    [Tile](const ViewState &ViewState) {
                        for (auto &&Child : Tile->Children)
                            if (!ViewState.frustum.IsBoxOutSideFrustumFast(Child->BoundingVolume))
                                return true;
                        return false;
                    }))
            {
                return false;
            }
        }
        else 
        {
            if (std::any_of(ViewStates.begin(), ViewStates.end(), 
                    [Tile](const ViewState &ViewState) {
                        return !ViewState.frustum.IsBoxOutSideFrustumFast(Tile->BoundingVolume);
                    }))
            {
                return false;
            }
        }
        return true;
    }

    bool UCesium3DTilesetComponent::MeetsSSE(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        double MaximumScreenSpaceError = GetMaxScreenSpaceError();

        double largestSSE = 0;
        for (auto &ViewState : ViewStates)
        {
            double distance = glm::distance(ViewState.cameraPosition, Tile->BoundingVolume.Center); 
            double sse = Tile->GeometricError * ViewState.resolution.y / (distance * ViewState.sseDenominator);
            largestSSE = glm::max(largestSSE, sse);
        }
        return largestSSE <= MaximumScreenSpaceError;
    }

    class FCesium3DTilesetSceneProxy : public FPrimitiveSceneProxy
    {
    public:

        FCesium3DTilesetSceneProxy(UCesium3DTilesetComponent *InComponent)
            : FPrimitiveSceneProxy(InComponent)
        {
            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            BuildCuboidVerts(2, 2, 2, OutVerts, OutIndices);
            IndexBuffer.Init(OutIndices);
            VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);
            BeginInitResource(&IndexBuffer);
            PreRenderHandle = GetAppication()->GetPreRenderDelegate().Add(this, &FCesium3DTilesetSceneProxy::PreRenderCallBack);
            PostRenderHandle = GetAppication()->GetPostRenderDelegate().Add(this, &FCesium3DTilesetSceneProxy::PostRenderCallBack);
            WireframeMaterial = dynamic_cast<UMaterial*>(GetContentManager()->GetContentByPath("/WireframeMaterial.json"));
        }

        void AddRenderingTiles(std::vector<Cesium3DTile *> Tiles)
        {
            std::unique_lock<std::mutex> lock(mutex);
            TilesRenderingQueue.push(Tiles);
        }

        void SetEcefToAbs(const dmat4 &InEcefToAbs)
        {
            EcefToAbs = InEcefToAbs;
        }

        void SetBoundingBoxVisibility(bool InShowBoundingBox)
        {
            this->bShowBoundingBox = InShowBoundingBox;
        }

        void PreRenderCallBack(FDynamicRHI*)
        {
            std::unique_lock<std::mutex> lock(mutex);
            TilesToRenderThisFrame = TilesRenderingQueue.front(); TilesRenderingQueue.pop();
            lock.unlock();
            for (auto iter = TilesToRenderThisFrame.begin(); iter != TilesToRenderThisFrame.end();)
            {
                Cesium3DTile *Tile = *iter;
                bool locked = Tile->mutex.try_lock();
                if (!locked)
                {
                    iter = TilesToRenderThisFrame.erase(iter);
                }
                else 
                {
                    iter++;
                }
            }
        }

        void PostRenderCallBack(FDynamicRHI*)
        {
            for (Cesium3DTile *Tile : TilesToRenderThisFrame)
            {
                Tile->mutex.unlock();
            }
        }

        virtual void DestroyRenderThreadResources() override
        {
            GetAppication()->GetPreRenderDelegate().Remove(PreRenderHandle);
            GetAppication()->GetPostRenderDelegate().Remove(PostRenderHandle);
            for (Cesium3DTile *Tile : TilesToRenderThisFrame)
            {
                Tile->mutex.unlock();
            }
            FPrimitiveSceneProxy::DestroyRenderThreadResources();
        }

        virtual void GetDynamicMeshElements(const std::vector<FViewSceneInfo*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    for (Cesium3DTile *Tile : TilesToRenderThisFrame)
                    {
                        dmat4 AxisTransform = dmat4(1);
                        if (Tile->TileGltfUpAxis == ETileGltfUpAxis::Y)
                            AxisTransform = Y_UP_TO_Z_UP;
                        else if (Tile->TileGltfUpAxis == ETileGltfUpAxis::X)
                            AxisTransform = X_UP_TO_Z_UP;
                        
                        dmat4 RtcCenterMatrix = dmat4(
                            dvec4(1, 0, 0, 0), 
                            dvec4(0, 1, 0, 0),
                            dvec4(0, 0, 1, 0),
                            dvec4(Tile->RtcCenter, 1)
                        );
                        Tile->TransformRHI->Data.LocalToWorld = GetLocalToWorld() * EcefToAbs * Tile->Transform * RtcCenterMatrix * AxisTransform;
                        if (!Tile->TransformRHI->IsInitialized())
                            BeginInitResource(Tile->TransformRHI.get());
                        else
                            Tile->TransformRHI->UpdateUniformBuffer();
                        for (std::shared_ptr<UStaticMesh> StaticMesh : Tile->Content.Gltf.StaticMeshes)
                        {
                            const FStaticMeshLODResources& LODModel = *StaticMesh->RenderData->LODResources[0];
                            for (int SectionIndex = 0; SectionIndex < LODModel.Sections.size(); SectionIndex++)
                            {
                                const FStaticMeshSection &Section = *LODModel.Sections[SectionIndex].get();
                                FMeshBatch Mesh;
                                Mesh.CastShadow = Section.bCastShadow;
                                Mesh.Element.VertexFactory = &Section.VertexFactory;
                                Mesh.Element.IndexBuffer = &Section.IndexBuffer;
                                Mesh.Element.NumVertices = Section.IndexBuffer.NumIndices;
                                Mesh.MaterialRenderProxy = StaticMesh->MaterialSlots[Section.MaterialIndex]->GetResource()->CreateRenderProxy();
                                Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", Tile->TransformRHI.get());
                                Collector.AddMesh(ViewIndex, Mesh);
                                if (bShowBoundingBox)
                                {
                                    if (TileBoundingBoxUBO.find(Tile) == TileBoundingBoxUBO.end())
                                    {
                                        TileBoundingBoxUBO[Tile] = CreateUniformBuffer<FPrimitiveShaderParameters>();
                                        BeginInitResource(TileBoundingBoxUBO[Tile].get());
                                    }
                                    dmat4 translation;
                                    translation = glm::translate(translation, Tile->BoundingVolume.Center);
                                    TUniformBufferRef<FPrimitiveShaderParameters> UBO = TileBoundingBoxUBO[Tile];
                                    UBO->Data.LocalToWorld = GetLocalToWorld() * EcefToAbs * translation * dmat4(Tile->BoundingVolume.HalfAxes);
                                    UBO->UpdateUniformBuffer();
                                    FMeshBatch DebugBoundingBoxMesh;
                                    DebugBoundingBoxMesh.CastShadow = Section.bCastShadow;
                                    DebugBoundingBoxMesh.Element.VertexFactory = &VertexFactory;
                                    DebugBoundingBoxMesh.Element.IndexBuffer = &IndexBuffer;
                                    DebugBoundingBoxMesh.Element.NumVertices = IndexBuffer.NumIndices;
                                    DebugBoundingBoxMesh.MaterialRenderProxy = WireframeMaterial->GetResource()->CreateRenderProxy();
                                    DebugBoundingBoxMesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", TileBoundingBoxUBO[Tile].get());
                                    Collector.AddMesh(ViewIndex, DebugBoundingBoxMesh);
                                }
                            }
                        }
                    }
                }
            }
        }

    protected:
        vector<Cesium3DTile *> TilesToRenderThisFrame;
        std::queue<std::vector<Cesium3DTile *>> TilesRenderingQueue;

        dmat4 EcefToAbs;

        bool bShowBoundingBox;
        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FStaticVertexFactory VertexFactory;
        std::unordered_map<Cesium3DTile *, TUniformBufferRef<FPrimitiveShaderParameters>> TileBoundingBoxUBO;

    private:

        std::mutex mutex;
        FDelegateHandle PreRenderHandle;
        FDelegateHandle PostRenderHandle;

        UMaterial *WireframeMaterial;

    };

    UCesium3DTilesetComponent::UCesium3DTilesetComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
        , URI("")
        , Georeference(nullptr)
        , MaximumScreenSpaceError(16.0)
        , bEnableFrustumCulling(true)
        , Tileset(nullptr)
        , bShowBoundingBox(false)
        , TilesetForSelection(nullptr)
    {
    }

    void UCesium3DTilesetComponent::TickComponent(double DeltaTime)
    {
        UWorld *World = GetWorld();
        if (World)
        {
            std::vector<ViewState> ViewStates;
            for (AActor *Actor : World->CameraActors)
            {
                std::vector<UCameraComponent*> CameraComponents;
                Actor->GetComponents(CameraComponents);
                for (UCameraComponent *CameraComponent : CameraComponents)
                {
                    ViewState viewState;
                    const dmat4 &AbsToEcef = Georeference->GetAbsToEcef();
                    viewState.cameraPosition = AbsToEcef * dvec4(CameraComponent->GetComponentLocation(), 1.0);
                    dvec3 forward = AbsToEcef * dvec4(CameraComponent->GetForwardVector(), 0.0);
                    dvec3 up = AbsToEcef * dvec4(CameraComponent->GetUpVector(), 0.0);
                    viewState.frustum = FViewFrustum(
                        viewState.cameraPosition, forward, up, 
                        CameraComponent->GetAspectRatio(), CameraComponent->GetFieldOfView(), 
                        CameraComponent->GetNearClipDistance(), CameraComponent->GetFarClipDistance());
                    viewState.resolution = CameraComponent->GetCameraResolution();
                    viewState.verticalFOV = CameraComponent->GetFieldOfView();
                    viewState.sseDenominator = 2.0 * glm::tan(0.5 * viewState.verticalFOV);
                    ViewStates.push_back(viewState);
                }
            }

            if (TilesetForSelection)
            {
                Update(TilesetForSelection.get(), ViewStates);
            }
            else 
            {
                TilesToRenderThisFrame.clear();
            }

            MarkRenderDynamicDataDirty();
        }
    }

    void UCesium3DTilesetComponent::SendRenderDynamicData()
    {
        FCesium3DTilesetSceneProxy *proxy = static_cast<FCesium3DTilesetSceneProxy *>(SceneProxy);
        proxy->SetBoundingBoxVisibility(bShowBoundingBox);
        if (Georeference)
            proxy->SetEcefToAbs(Georeference->GetEcefToAbs());
        if (TilesetForSelection)
            proxy->AddRenderingTiles(TilesToRenderThisFrame);
        UPrimitiveComponent::SendRenderDynamicData();
    }

    FPrimitiveSceneProxy *UCesium3DTilesetComponent::CreateSceneProxy()
    {
        return new FCesium3DTilesetSceneProxy(this);
    }

    void UCesium3DTilesetComponent::OnRegister()
    {
        if (Georeference == nullptr && WorldPrivate != nullptr)
        {
            std::vector<AGeoreferenceActor*> Georeferences;
            WorldPrivate->GetAllActorsOfClass<AGeoreferenceActor>(Georeferences, false);
            if (!Georeferences.empty())
            {
                this->Georeference = Georeferences[0];
            }
            else 
            {
                this->Georeference = WorldPrivate->SpawnActor<AGeoreferenceActor>(FTransform::Identity, "CesiumGeoreference").get();
            }
        }

        UPrimitiveComponent::OnRegister();
    }

    FBoundingBox UCesium3DTilesetComponent::CalcBounds(const FTransform &LocalToWorld) const
    {
        if (TilesetForSelection)
        {
            dvec3 center = LocalToWorld.TransformPosition(TilesetForSelection->Root->BoundingVolume.Center);
            dvec3 xDirection = LocalToWorld.TransformVector(TilesetForSelection->Root->BoundingVolume.HalfAxes[0]);
            dvec3 yDirection = LocalToWorld.TransformVector(TilesetForSelection->Root->BoundingVolume.HalfAxes[1]);
            dvec3 zDirection = LocalToWorld.TransformVector(TilesetForSelection->Root->BoundingVolume.HalfAxes[2]);

            return FBoundingBox(center, xDirection, yDirection, zDirection);
        }
        return UPrimitiveComponent::CalcBounds(LocalToWorld);
    }

    void UCesium3DTilesetComponent::SetURI(const std::string &NewURI)
    { 
        URI = NewURI; 
        tiny3dtiles::Loader Loader;
        Tileset = Loader.LoadTileset(URI);
        if (Tileset)
            TilesetForSelection = Cesium3DTileset::Build(Tileset, dmat4(1));
        else
            TilesetForSelection = nullptr;
    }

    void UCesium3DTilesetComponent::SetShowBoundingBox(bool InShowBoundingBox)
    {
        bShowBoundingBox = InShowBoundingBox;
        MarkRenderDynamicDataDirty();
    }

    void UCesium3DTilesetComponent::SetGeoreference(AGeoreferenceActor *InGeoreference)
    {
        Georeference = InGeoreference;
        MarkRenderDynamicDataDirty();
    }
}