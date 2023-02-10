#include "ContentManager.h"
#include "Texture.h"

#include "Common/Log.h"

namespace nilou {


    FContentManager &FContentManager::GetContentManager()
    {
        static FContentManager GContentManager;
        return GContentManager;
    }

    void FContentManager::AddGlobalTexture(const std::string &name, std::shared_ptr<FTexture> texture, bool overlap)
    {
        if (!GlobalTextures.Insert(name, texture, overlap))
            NILOU_LOG(Error, "FContentManager::AddGlobalTexture: Texture \"" + name + "\" already exists, but overlap parameter is set to false");
    }
    void FContentManager::RemoveGlobalTexture(const std::string &name)
    {
        GlobalTextures.Erase(name);
    }
    FTexture *FContentManager::GetGlobalTexture(const std::string &name)
    {
        FTexture *Texture = GlobalTextures.Get(name).get();
        if (Texture == nullptr)
            NILOU_LOG(Error, "FContentManager::GetGlobalTexture: Texture \"" + name + "\" doesn't exist");
        return Texture;
    }

    void FContentManager::AddGlobalMaterial(const std::string &name, std::shared_ptr<FMaterial> material, bool overlap)
    {
        if (!GlobalMaterials.Insert(name, material, overlap))
            NILOU_LOG(Error, "FContentManager::AddGlobalMaterial: Material \"" + name + "\" already exists, but overlap parameter is set to false");
    }
    void FContentManager::RemoveGlobalMaterial(const std::string &name)
    {
        GlobalMaterials.Erase(name);
    }
    FMaterial *FContentManager::GetGlobalMaterial(const std::string &name)
    {
        FMaterial *Material = GlobalMaterials.Get(name).get();
        if (Material == nullptr)
            NILOU_LOG(Error, "FContentManager::GetGlobalMaterial: Material \"" + name + "\" doesn't exist");
        return Material;
    }

    void FContentManager::AddGlobalStaticMesh(const std::string &name, std::shared_ptr<UStaticMesh> mesh, bool overlap)
    {
        if (!GlobalStaticMeshes.Insert(name, mesh, overlap))
            NILOU_LOG(Error, "FContentManager::AddGlobalStaticMesh: StaticMesh \"" + name + "\" already exists, but overlap parameter is set to false");
    }
    void FContentManager::RemoveGlobalStaticMesh(const std::string &name)
    {
        GlobalStaticMeshes.Erase(name);
    }
    std::shared_ptr<UStaticMesh> FContentManager::GetGlobalStaticMesh(const std::string &name)
    {
        std::shared_ptr<UStaticMesh> Mesh = GlobalStaticMeshes.Get(name);
        if (Mesh == nullptr)
            NILOU_LOG(Error, "FContentManager::GetGlobalStaticMesh: StaticMesh \"" + name + "\" doesn't exist");
        return Mesh;
    }
}