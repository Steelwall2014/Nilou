#include <fstream>

#include "Common/StaticMeshResources.h"
#include "Material.h"
#include "Common/World.h"
#include "Cesium3DTilesetComponent.h"


namespace nilou {

    using namespace Cesium3DTilesetSelection;

    void BoundingVolumeConvert(FOrientedBoundingBox &Box, tiny3dtiles::BoundingVolume &BoundingVolume, const glm::dmat4 &transform)
    {
        auto &box = BoundingVolume.box;
        dvec3 center = transform * dvec4(box[0], box[1], box[2], 1.0);
        dmat3 halfAxis = dmat3(transform) * dmat3(box[3], box[4], box[5], box[6], box[7], box[8], box[9], box[10], box[11]);
        Box = FOrientedBoundingBox(center, halfAxis);
    }

    std::shared_ptr<Cesium3DTile> Cesium3DTile::BuildTile(
        std::shared_ptr<tiny3dtiles::Tile> Tile, const glm::dmat4 &parentTransform)
    {
        std::shared_ptr<Cesium3DTile> InternalTile = std::make_shared<Cesium3DTile>();
        InternalTile->GeometricError = Tile->geometricError;
        InternalTile->Refine = Tile->refine;
        InternalTile->Transform = Tile->transform * parentTransform;
        BoundingVolumeConvert(InternalTile->BoundingVolume, Tile->boundingVolume, InternalTile->Transform);
        InternalTile->Content.URI = Tile->content.uri;
        InternalTile->TransformRHI = CreateUniformBuffer<FPrimitiveShaderParameters>();
        InternalTile->TransformRHI->InitRHI();

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
            InternalTile->Children.push_back(BuildTile(child, InternalTile->Transform));
        }

        return InternalTile;
    }

    std::shared_ptr<Cesium3DTileset> Cesium3DTileset::Build(
        std::shared_ptr<tiny3dtiles::Tileset> Tileset, const glm::dmat4 &parentTransform)
    {
        std::shared_ptr<Cesium3DTileset> InternalTileset = std::make_shared<Cesium3DTileset>();
        InternalTileset->Asset = Tileset->asset;
        InternalTileset->GeometricError = Tileset->geometricError;
        InternalTileset->Root = Cesium3DTile::BuildTile(Tileset->root, parentTransform);
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
            m.lock();
            MainThreadTaskQueue.pop();
            m.unlock();
        }
    }

    void UCesium3DTilesetComponent::Update(Cesium3DTileset *Tileset, const std::vector<ViewState> &ViewStates)
    {
        // TilesToRenderThisFrame.clear();

        DispatchMainThreadTask();

        UpdateInternal(Tileset->Root.get(), ViewStates);

        UnloadTiles();

        LoadTiles();

        // return TilesToRenderThisFrame;
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
                    if (Tile->Content.B3dm)
                    {
                        std::ifstream stream(Tile->Content.URI, std::ios::binary);
                        stream >> *Tile->Content.B3dm;
                        tinygltf::TinyGLTF Loader;
                        std::string err;
                        std::string warn;
                        Loader.LoadBinaryFromMemory(result.model.get(), &err, &warn, Tile->Content.B3dm->glb.data(), Tile->Content.B3dm->glb.size());
                    }
                    return result;
                };
        std::queue<TileMainThreadTask> &TaskQueue = MainThreadTaskQueue;
        std::mutex &mutex = m;
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
            if (Tile->Content.B3dm)
            {
                Tile->Content.B3dm->reset();
            }
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
                if (Tile->Content.B3dm == nullptr)
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

    void LRUCache::Load(Cesium3DTile *Tile)
    {
        if (Tile->LoadingState == ETileLoadingState::Loaded)
        {
            LoadedTiles.splice(LoadedTiles.begin(), LoadedTiles, Tile->iter);
            return;
        }

        if (LoadedTiles.size() >= Capacity)
        {
            auto iter = LoadedTiles.end();
            iter--;

            Cesium3DTile *LRUTile = *iter;
            LRUTile->LoadingState = ETileLoadingState::Unloading;
            LRUTile->Content.B3dm = nullptr;
            LoadedTiles.erase(iter);
            LRUTile->LoadingState = ETileLoadingState::Unloaded;
        }

        Tile->LoadingState = ETileLoadingState::Loading;
        Tile->Content.B3dm = std::make_shared<tiny3dtiles::B3DM>();
        std::ifstream stream(Tile->Content.URI, std::ios::binary);
        stream >> *Tile->Content.B3dm;
        Tile->LoadingState = ETileLoadingState::Loaded;
        Tile->iter = LoadedTiles.insert(LoadedTiles.begin(), Tile);
    }

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
        // TilesToRenderThisFrame.push_back(Tile);
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
                        return !ViewState.frustum.IsBoxOutSideFrustum(Tile->BoundingVolume);
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

        }

        void SetRenderTiles(std::vector<Cesium3DTile *> Tiles)
        {
            TilesToRenderThisFrame = Tiles;
        }

        void SetEcefToAbs(const dmat4 &InEcefToAbs)
        {
            EcefToAbs = InEcefToAbs;
        }

        virtual void GetDynamicMeshElements(const std::vector<const FSceneView *> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    for (Cesium3DTile *Tile : TilesToRenderThisFrame)
                    {
                        if (Tile->LoadingState != ETileLoadingState::Loaded)
                            continue;

                        Tile->TransformRHI->Data.LocalToWorld = GetLocalToWorld() * EcefToAbs * Tile->Transform;
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
                            }
                        }
                    }
                }
            }
        }

    protected:

        std::vector<Cesium3DTile *> TilesToRenderThisFrame;

        dmat4 EcefToAbs;

    };

    UCesium3DTilesetComponent::UCesium3DTilesetComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
        // , TileSelector(this)
        , URI("")
        , Georeference(nullptr)
        , MaximumScreenSpaceError(16.0)
        , bEnableFrustumCulling(true)
        , Tileset(nullptr)
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
                    ViewState ViewState;
                    const dmat4 &AbsToEcef = Georeference->GetAbsToEcef();
                    ViewState.cameraPosition = AbsToEcef * dvec4(CameraComponent->GetComponentLocation(), 1.0);
                    dvec3 forward = AbsToEcef * dvec4(CameraComponent->GetForwardVector(), 0.0);
                    dvec3 up = AbsToEcef * dvec4(CameraComponent->GetUpVector(), 0.0);
                    ViewState.frustum = FViewFrustum(
                        ViewState.cameraPosition, forward, up, 
                        CameraComponent->GetAspectRatio(), CameraComponent->GetFieldOfView(), 
                        CameraComponent->GetNearClipDistance(), CameraComponent->GetFarClipDistance());
                    ViewState.resolution = CameraComponent->GetCameraResolution();
                    ViewState.verticalFOV = CameraComponent->GetFieldOfView();
                    ViewState.sseDenominator = 2.0 * glm::tan(0.5 * ViewState.verticalFOV);
                    ViewStates.push_back(ViewState);
                }
            }

            Update(TilesetForSelection.get(), ViewStates);
            // std::vector<RendableTileContent> RenderTileMeshes;
            // for (Cesium3DTile *RenderTile : RenderTiles)
            // {
            //     for (std::shared_ptr<UStaticMesh> StaticMesh : RenderTile->Content.Gltf.StaticMeshes)
            //     {
            //         RendableTileContent content;
            //         content.StaticMesh = StaticMesh;
            //         content.TransformRHI = CreateUniformBuffer<FPrimitiveShaderParameters>();
            //         content.TransformRHI->Data.LocalToWorld = RenderTile->Transform;
            //         RenderTileMeshes.push_back(content);
            //     }
            // }
            FCesium3DTilesetSceneProxy *Proxy = static_cast<FCesium3DTilesetSceneProxy *>(SceneProxy);
            Proxy->SetRenderTiles(TilesToRenderThisFrame);
            Proxy->SetEcefToAbs(Georeference->GetEcefToAbs());
        }
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
        TilesetForSelection = Cesium3DTileset::Build(Tileset, glm::dmat4(1));
    }
}