#pragma once
#include <variant>

#include <tiny3dtiles/tiny_3dtiles.h>
#include <tinygltf/tiny_gltf.h>

#include "PrimitiveComponent.h"

#include "Common/Actor/GeoreferenceActor.h"

namespace nilou {
    class UCesium3DTilesetComponent;

    namespace Cesium3DTilesetSelection {

    struct ViewState
    {
        FViewFrustum frustum;
        dvec3 cameraPosition;
        ivec2 resolution;
        double verticalFOV;
        /** The denominator for calculating screen space error */
        double sseDenominator; 
    };

    class Cesium3DTile
    {
    public:
        friend class LRUCache;
        friend class Cesium3DTilesetSelector;
        struct TileContent
        {
            std::string URI;
            std::shared_ptr<tiny3dtiles::B3DM> B3dm = nullptr;
            GLTFParseResult Gltf;
        };
        tiny3dtiles::Tile::Refinement Refine = tiny3dtiles::Tile::Refinement::REFINE;
        TileContent Content;
        std::vector<std::shared_ptr<Cesium3DTile>> Children;
        FOrientedBoundingBox BoundingVolume;    //  The bounding volume that has been transformed by parent transforms
        double GeometricError;
        glm::dmat4 Transform = glm::dmat4(1);   // The matrix that has been multiplied with parent transforms
    
        static std::shared_ptr<Cesium3DTile> BuildTile(
            std::shared_ptr<tiny3dtiles::Tile> Tile, const glm::dmat4 &parentTransform);
    private:
        std::list<Cesium3DTile *>::iterator iter;
        bool bLoaded = false;
        bool IsLeaf() const { return Children.empty(); }
    };

    class Cesium3DTileset
    {
    public:
        std::map<std::string, std::string> Asset;
        std::shared_ptr<Cesium3DTile> Root;
        double GeometricError;

        static std::shared_ptr<Cesium3DTileset> Build(
            std::shared_ptr<tiny3dtiles::Tileset> Tileset, const glm::dmat4 &parentTransform);
    };

    class LRUCache
    {
    public:
        LRUCache() { }

        LRUCache(int Capacity);

        void Load(Cesium3DTile *Tile);

        int Capacity;

    private:

        std::list<Cesium3DTile *> LoadedTiles;

    };

    class Cesium3DTilesetSelector
    {
    public:
        Cesium3DTilesetSelector(UCesium3DTilesetComponent *InComponent): Component(InComponent) { }

        const std::vector<Cesium3DTile *> &Update(Cesium3DTileset *Tileset, const std::vector<ViewState> &ViewStates);
    
    private:
        void UpdateInternal(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates);

        void AddTileToUnloadQueue(Cesium3DTile *Tile);

        void AddTileToRenderQueue(Cesium3DTile *Tile);

        void LoadTiles();

        void UnloadTiles();

        /** Return true if tile is culled */
        bool FrustumCull(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates);

        /** 
         * @brief Return true if the largest sse of this tile in all frustums is smaller than MaximumScreenSpaceError. 
         * For screen space error, see "Level of Detail For 3D Graphics"
         */
        bool MeetsSSE(Cesium3DTile *Tile, const std::vector<ViewState> &ViewStates);

        // std::set<Cesium3DTile *> TilesToRenderLastFrame;
        std::vector<Cesium3DTile *> TilesToRenderThisFrame;

        std::vector<Cesium3DTile *> LoadQueue;
        std::vector<Cesium3DTile *> UnloadQueue;

        UCesium3DTilesetComponent *Component;

        // LRUCache Cache;
    };

    }   // namespace Cesium3DTilesetSelection


    UCLASS()
    class UCesium3DTilesetComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:
        UCesium3DTilesetComponent(AActor *InOwner = nullptr);

        virtual void OnRegister() override;

        virtual void TickComponent(double DeltaTime) override;

        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;

        virtual FBoundingBox CalcBounds(const FTransform &LocalToWorld) const override;
        
        inline std::string GetURI() const { return URI; }
        void SetURI(const std::string &NewURI);
        inline double GetMaxScreenSpaceError() const { return MaximumScreenSpaceError; }
        inline void SetMaxScreenSpaceError(double NewMaxScreenSpaceError) { MaximumScreenSpaceError = NewMaxScreenSpaceError; }
        inline bool GetEnableFrustumCulling() const { return bEnableFrustumCulling; }
        inline void SetEnableFrustumCulling(bool NewEnableFrustumCulling) { bEnableFrustumCulling = NewEnableFrustumCulling; }

    protected:

        std::string URI;

        AGeoreferenceActor *Georeference;

        double MaximumScreenSpaceError;

        bool bEnableFrustumCulling;

        std::shared_ptr<tiny3dtiles::Tileset> Tileset;

        void UpdateView(const std::vector<FViewFrustum> &Frustums);

    private:
        std::shared_ptr<Cesium3DTilesetSelection::Cesium3DTileset> TilesetForSelection;

        Cesium3DTilesetSelection::Cesium3DTilesetSelector TileSelector;
    };




}