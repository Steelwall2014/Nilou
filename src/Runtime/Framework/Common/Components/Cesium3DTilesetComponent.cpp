#include <fstream>
#include <sstream>

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
        InternalTile->TransformRHI->InitRHI();
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

    void UCesium3DTilesetComponent::DispatchMainThreadTask()
    {
        int TaskCount = MainThreadTaskQueue.size();
        for (int TaskIndex = 0; TaskIndex < TaskCount; TaskIndex++)
        {
            TileMainThreadTask &Task = MainThreadTaskQueue.front();
            Task.Func(Task.Result);
            mutex.lock();
            MainThreadTaskQueue.pop();
            mutex.unlock();
        }
    }

    void UCesium3DTilesetComponent::Update(Cesium3DTileset *Tileset, const std::vector<ViewState> &ViewStates)
    {
        TilesToRenderThisFrame.clear();

        DispatchMainThreadTask();

        UpdateInternal(Tileset->Root.get(), ViewStates);

        UnloadTiles();

        LoadTiles();
    }

    void UCesium3DTilesetComponent::LoadContent(
        Cesium3DTile *Tile, 
        std::function<void(TileLoadingResult)> MainThreadFunc)
    {
        auto LoadFunc = [](Cesium3DTile *Tile) {
                    Tile->LoadingState = ETileLoadingState::Loading;
                    TileLoadingResult result;
                    result.model = std::make_shared<tinygltf::Model>();
                    result.Tile = Tile;
                    if (Tile->Content.B3dm.header_loaded())
                    {
                        Tile->Content.B3dm.load_glb(Tile->Content.URI);
                        tinygltf::TinyGLTF Loader;
                        std::string err;
                        std::string warn;
                        Loader.LoadBinaryFromMemory(result.model.get(), &err, &warn, Tile->Content.B3dm.glb.data(), Tile->Content.B3dm.glb.size());
                        if (err != "")
                            std::cout << err;
                    }
                    return result;
                };
        std::queue<TileMainThreadTask> &TaskQueue = MainThreadTaskQueue;
        std::mutex &mutex = this->mutex;
        pool.push_task([&mutex, LoadFunc, Tile, &TaskQueue, MainThreadFunc]() {
            TileMainThreadTask ThreadTask;
            ThreadTask.Result = LoadFunc(Tile);
            ThreadTask.Func = MainThreadFunc;
            mutex.lock();
            TaskQueue.push(ThreadTask);
            mutex.unlock();
        });
    }

    void UCesium3DTilesetComponent::LoadTiles()
    {
        for (Cesium3DTile *Tile : LoadQueue)
        {
            LoadContent(Tile, [this](TileLoadingResult Result) { 
                if (Result.model->meshes.size() != 0)
                {
                    Result.Tile->Content.Gltf = GameStatics::ParseToStaticMeshes(*Result.model);
                    for (std::shared_ptr<FMaterial> Material : Result.Tile->Content.Gltf.Materials) {
                        Material->RasterizerState.CullMode = ERasterizerCullMode::CM_CCW;
                    }
                    Result.Tile->LoadingState = ETileLoadingState::Loaded;
                    TilesToRenderThisFrame.push_back(Result.Tile);
                }
            });
        }
        LoadQueue.clear();
    }

    void UCesium3DTilesetComponent::UnloadTiles()
    {
        for (Cesium3DTile *Tile : UnloadQueue)
        {
            if (Tile->LoadingState != ETileLoadingState::Loaded)
                continue;
            Tile->LoadingState = ETileLoadingState::Unloading;
            Tile->Content.B3dm.reset_glb();
            Tile->Content.Gltf.Materials.clear();
            Tile->Content.Gltf.Textures.clear();
            Tile->Content.Gltf.StaticMeshes.clear();
            Tile->LoadingState = ETileLoadingState::Unloaded;
        }
        UnloadQueue.clear();
    }

    void UCesium3DTilesetComponent::UpdateInternal(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        bool bTileCulled = FrustumCull(Tile, ViewStates);

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
        {
            UnloadQueue.push_back(Tile);
        }
    }

    void UCesium3DTilesetComponent::AddTileToRenderQueue(Cesium3DTile *Tile)
    {
        if (Tile->LoadingState == ETileLoadingState::Unloaded)
        {
            LoadQueue.push_back(Tile);
        }
        else 
        {
            TilesToRenderThisFrame.push_back(Tile);
        }
    }

    bool isVisibleFromCamera(const FViewFrustum &frustum, FOrientedBoundingBox)
    {
        
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
            {
                BeginInitResource(&IndexBuffer);
            }
        }

        void SetRenderTiles(std::vector<Cesium3DTile *> Tiles)
        {
            TilesToRenderThisFrame = Tiles;
        }

        void SetEcefToAbs(const dmat4 &InEcefToAbs)
        {
            EcefToAbs = InEcefToAbs;
        }

        void SetBoundingBoxVisibility(bool InShowBoundingBox)
        {
            this->bShowBoundingBox = InShowBoundingBox;
        }

        virtual void GetDynamicMeshElements(const std::vector<FViewSceneInfo*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    for (Cesium3DTile *Tile : TilesToRenderThisFrame)
                    {
                        if (Tile->LoadingState != ETileLoadingState::Loaded)
                            continue;
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
                                Mesh.MaterialRenderProxy = StaticMesh->MaterialSlots[Section.MaterialIndex];
                                Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", Tile->TransformRHI.get());
                                Collector.AddMesh(ViewIndex, Mesh);
                                if (bShowBoundingBox)
                                {
                                    if (TileBoundingBoxUBO.find(Tile) == TileBoundingBoxUBO.end())
                                    {
                                        TileBoundingBoxUBO[Tile] = CreateUniformBuffer<FPrimitiveShaderParameters>();
                                        TileBoundingBoxUBO[Tile]->InitRHI();
                                    }
                                    dmat4 translation;
                                    translation = glm::translate(translation, Tile->BoundingVolume.Center);
                                    TileBoundingBoxUBO[Tile]->Data.LocalToWorld = GetLocalToWorld() * EcefToAbs * translation * dmat4(Tile->BoundingVolume.HalfAxes);
                                    TileBoundingBoxUBO[Tile]->UpdateUniformBuffer();
                                    FMeshBatch DebugBoundingBoxMesh;
                                    DebugBoundingBoxMesh.CastShadow = Section.bCastShadow;
                                    DebugBoundingBoxMesh.Element.VertexFactory = &VertexFactory;
                                    DebugBoundingBoxMesh.Element.IndexBuffer = &IndexBuffer;
                                    DebugBoundingBoxMesh.Element.NumVertices = IndexBuffer.NumIndices;
                                    DebugBoundingBoxMesh.MaterialRenderProxy = FContentManager::GetContentManager().GetGlobalMaterial("WireframeMaterial");
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

        std::vector<Cesium3DTile *> TilesToRenderThisFrame;

        dmat4 EcefToAbs;

        bool bShowBoundingBox;
        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FStaticVertexFactory VertexFactory;
        std::unordered_map<Cesium3DTile *, TUniformBufferRef<FPrimitiveShaderParameters>> TileBoundingBoxUBO;


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
            proxy->SetRenderTiles(TilesToRenderThisFrame);
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