#pragma once
#include <variant>

#include <tiny3dtiles/tiny_3dtiles.h>
#include <tinygltf/tiny_gltf.h>

#include "Common/Actor/GeoreferenceActor.h"
#include "Common/Actor/Actor.h"

#include "thread_pool/BS_thread_pool.hpp"
#include "Common/LruCache.h"

#include "Cesium3DTilesetSelection.h"


namespace nilou {

    UCLASS()
    class ACesium3DTileset : public AActor
    {
        GENERATE_CLASS_INFO()

    public:

        virtual void Tick(double DeltaSeconds) override;

        uint32 GetMaxTilesToRender() const { return MaxTilesToRender; }

        void SetMaxTilesToRender(uint32 InMaxTilesToRender) 
        { 
            MaxTilesToRender = InMaxTilesToRender; 
            LruCache.SetCapacity(InMaxTilesToRender); 
        }
        
        std::string GetURI() const { return URI; }

        void SetURI(const std::string &NewURI);

        ACesium3DTileset();

        AGeoreferenceActor *Georeference;

        double MaximumScreenSpaceError = 16.0;

        bool bEnableFrustumCulling = true;

        bool bShowBoundingBox = false;

    protected:

        std::string URI;

        uint32 MaxTilesToRender = 128;

        std::shared_ptr<tiny3dtiles::Tileset> Tileset;

        std::shared_ptr<Cesium3DTilesetSelection::Cesium3DTileset> TilesetForSelection;

        std::set<Cesium3DTilesetSelection::Cesium3DTile *> TilesToRenderThisFrame;
        std::vector<std::shared_ptr<class UPrimitiveComponent>> RenderComponentsThisFrame;

        TLruCache<Cesium3DTilesetSelection::Cesium3DTile*, Cesium3DTilesetSelection::Cesium3DTile*> LruCache;

        BS::thread_pool pool;

        void Update(Cesium3DTilesetSelection::Cesium3DTileset *Tileset, const std::vector<Cesium3DTilesetSelection::ViewState> &ViewStates);

        void UpdateInternal(Cesium3DTilesetSelection::Cesium3DTile *Tile, const std::vector<Cesium3DTilesetSelection::ViewState> &ViewStates);

        /** Return true if tile is culled */
        bool FrustumCull(Cesium3DTilesetSelection::Cesium3DTile *Tile, const std::vector<Cesium3DTilesetSelection::ViewState> &ViewStates);

        /** 
         * @brief Return true if the largest sse of this tile in all frustums is smaller than MaximumScreenSpaceError. 
         * For screen space error, see "Level of Detail For 3D Graphics"
         */
        bool MeetsSSE(Cesium3DTilesetSelection::Cesium3DTile *Tile, const std::vector<Cesium3DTilesetSelection::ViewState> &ViewStates);
    };

}