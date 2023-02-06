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
        if (!GlobalTextures.Insert(name, texture))
            NILOU_LOG(Error, "Texture " + name + " doesn't exist");
    }
    void FContentManager::RemoveGlobalTexture(const std::string &name)
    {
        GlobalTextures.Erase(name);
    }
    FTexture *FContentManager::GetGlobalTexture(const std::string &name)
    {
        return GlobalTextures.Get(name).get();
    }

    void FContentManager::AddGlobalMaterial(const std::string &name, std::shared_ptr<FMaterial> material, bool overlap)
    {
        if (!GlobalMaterials.Insert(name, material))
            NILOU_LOG(Error, "Material " + name + " doesn't exist");
    }
    void FContentManager::RemoveGlobalMaterial(const std::string &name)
    {
        GlobalMaterials.Erase(name);
    }
    FMaterial *FContentManager::GetGlobalMaterial(const std::string &name)
    {
        return GlobalMaterials.Get(name).get();
    }

    void FContentManager::AddGlobalStaticMesh(const std::string &name, std::shared_ptr<UStaticMesh> mesh, bool overlap)
    {
        if (!GlobalStaticMeshes.Insert(name, mesh))
            NILOU_LOG(Error, "StaticMesh " + name + " doesn't exist");
    }
    void FContentManager::RemoveGlobalStaticMesh(const std::string &name)
    {
        GlobalStaticMeshes.Erase(name);
    }
    std::shared_ptr<UStaticMesh> FContentManager::GetGlobalStaticMesh(const std::string &name)
    {
        return GlobalStaticMeshes.Get(name);
    }
}