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

    const std::vector<Cesium3DTile *> &Cesium3DTilesetSelector::Update(Cesium3DTileset *Tileset, const std::vector<ViewState> &ViewStates)
    {
        TilesToRenderThisFrame.clear();

        UpdateInternal(Tileset->Root.get(), ViewStates);

        LoadTiles();

        UnloadTiles();

        return TilesToRenderThisFrame;
    }

    void Cesium3DTilesetSelector::LoadTiles()
    {
        for (Cesium3DTile *Tile : LoadQueue)
        {
            if (Tile->Content.B3dm)
            {
                std::ifstream stream(Tile->Content.URI, std::ios::binary);
                stream >> *Tile->Content.B3dm;
                tinygltf::TinyGLTF Loader;
                tinygltf::Model model;
                std::string err;
                std::string warn;
                Loader.LoadBinaryFromMemory(&model, &err, &warn, Tile->Content.B3dm->glb.data(), Tile->Content.B3dm->glb.size());
                Tile->Content.Gltf = GameStatics::ParseToStaticMeshes(model);
                for (std::shared_ptr<FMaterial> Material : Tile->Content.Gltf.Materials) {
                    Material->RasterizerState.CullMode = ERasterizerCullMode::CM_None;
                }
            }
            Tile->bLoaded = true;
        }
        LoadQueue.clear();
    }

    void Cesium3DTilesetSelector::UnloadTiles()
    {
        for (Cesium3DTile *Tile : UnloadQueue)
        {
            if (Tile->Content.B3dm)
            {
                Tile->Content.B3dm->reset();
            }
            Tile->Content.Gltf.Materials.clear();
            Tile->Content.Gltf.Textures.clear();
            Tile->Content.Gltf.StaticMeshes.clear();
            Tile->bLoaded = false;
        }
        UnloadQueue.clear();
    }

    void Cesium3DTilesetSelector::UpdateInternal(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
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
                    for (std::shared_ptr<Cesium3DTile> Tile : Tile->Children)
                    {
                        UpdateInternal(Tile.get(), ViewStates);
                    }
                }
            }
            else 
            {
                if (Tile->Content.B3dm == nullptr)
                {
                    AddTileToUnloadQueue(Tile);
                    for (std::shared_ptr<Cesium3DTile> Tile : Tile->Children)
                    {
                        UpdateInternal(Tile.get(), ViewStates);
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
        if (Tile->bLoaded)
        {
            LoadedTiles.splice(LoadedTiles.begin(), LoadedTiles, Tile->iter);
            return;
        }

        if (LoadedTiles.size() >= Capacity)
        {
            auto iter = LoadedTiles.end();
            iter--;

            Cesium3DTile *LRUTile = *iter;
            LRUTile->Content.B3dm = nullptr;
            LRUTile->bLoaded = false;

            LoadedTiles.erase(iter);
        }

        Tile->Content.B3dm = std::make_shared<tiny3dtiles::B3DM>();
        std::ifstream stream(Tile->Content.URI, std::ios::binary);
        stream >> *Tile->Content.B3dm;
        Tile->bLoaded = true;
        Tile->iter = LoadedTiles.insert(LoadedTiles.begin(), Tile);
    }

    void Cesium3DTilesetSelector::AddTileToUnloadQueue(Cesium3DTile *Tile)
    {
        if (Tile->bLoaded == true)
        {
            UnloadQueue.push_back(Tile);
        }
    }

    void Cesium3DTilesetSelector::AddTileToRenderQueue(Cesium3DTile *Tile)
    {
        if (Tile->bLoaded == false)
        {
            LoadQueue.push_back(Tile);
        }
        TilesToRenderThisFrame.push_back(Tile);
    }

    bool Cesium3DTilesetSelector::FrustumCull(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
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

    bool Cesium3DTilesetSelector::MeetsSSE(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates)
    {
        double MaximumScreenSpaceError = Component->GetMaxScreenSpaceError();

        double largestSSE = 0;
        for (auto &ViewState : ViewStates)
        {
            double distance = glm::distance(ViewState.cameraPosition, Tile->BoundingVolume.Center); 
            double sse = Tile->GeometricError * ViewState.resolution.y / (distance * ViewState.sseDenominator);
            largestSSE = glm::max(largestSSE, sse);
        }
        return largestSSE <= MaximumScreenSpaceError;
    }

    struct RendableTileContent
    {
        std::shared_ptr<UStaticMesh> StaticMesh;
        TUniformBufferRef<FPrimitiveShaderParameters> TransformUBO;
    };

    class FCesium3DTilesetSceneProxy : public FPrimitiveSceneProxy
    {
    public:

        FCesium3DTilesetSceneProxy(UCesium3DTilesetComponent *InComponent)
            : FPrimitiveSceneProxy(InComponent)
        {

        }

        void SetRenderTiles(const std::vector<RendableTileContent> Tiles)
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
                    for (auto &[StaticMesh, TransformUBO] : TilesToRenderThisFrame)
                    {
                        TransformUBO->Data.LocalToWorld = GetLocalToWorld() * EcefToAbs * TransformUBO->Data.LocalToWorld;
                        TransformUBO->InitRHI();
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
                            Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", TransformUBO.get());
                            Collector.AddMesh(ViewIndex, Mesh);
                        }
                    }
                }
            }
        }

    protected:

        std::vector<RendableTileContent> TilesToRenderThisFrame;

        dmat4 EcefToAbs;

    };

    UCesium3DTilesetComponent::UCesium3DTilesetComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
        , TileSelector(this)
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

            const std::vector<Cesium3DTile *> &RenderTiles = TileSelector.Update(TilesetForSelection.get(), ViewStates);
            std::vector<RendableTileContent> RenderTileMeshes;
            for (Cesium3DTile *RenderTile : RenderTiles)
            {
                for (std::shared_ptr<UStaticMesh> StaticMesh : RenderTile->Content.Gltf.StaticMeshes)
                {
                    RendableTileContent content;
                    content.StaticMesh = StaticMesh;
                    content.TransformUBO = CreateUniformBuffer<FPrimitiveShaderParameters>();
                    content.TransformUBO->Data.LocalToWorld = RenderTile->Transform;
                    RenderTileMeshes.push_back(content);
                }
            }
            FCesium3DTilesetSceneProxy *Proxy = static_cast<FCesium3DTilesetSceneProxy *>(SceneProxy);
            Proxy->SetRenderTiles(RenderTileMeshes);
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