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
        if (!GlobalTextures.insert(name, texture))
            NILOU_LOG(Error, "Texture " + name + " doesn't exist");
    }
    void FContentManager::RemoveGlobalTexture(const std::string &name)
    {
        GlobalTextures.erase(name);
    }
    std::shared_ptr<FTexture> FContentManager::GetGlobalTexture(const std::string &name)
    {
        return GlobalTextures.get(name);
    }

    void FContentManager::AddGlobalMaterial(const std::string &name, std::shared_ptr<FMaterial> material, bool overlap)
    {
        if (!GlobalMaterials.insert(name, material))
            NILOU_LOG(Error, "Material " + name + " doesn't exist");
    }
    void FContentManager::RemoveGlobalMaterial(const std::string &name)
    {
        GlobalMaterials.erase(name);
    }
    std::shared_ptr<FMaterial> FContentManager::GetGlobalMaterial(const std::string &name)
    {
        return GlobalMaterials.get(name);
    }

    void FContentManager::AddGlobalStaticMesh(const std::string &name, std::shared_ptr<UStaticMesh> mesh, bool overlap)
    {
        if (!GlobalStaticMeshes.insert(name, mesh))
            NILOU_LOG(Error, "StaticMesh " + name + " doesn't exist");
    }
    void FContentManager::RemoveGlobalStaticMesh(const std::string &name)
    {
        GlobalStaticMeshes.erase(name);
    }
    std::shared_ptr<UStaticMesh> FContentManager::GetGlobalStaticMesh(const std::string &name)
    {
        return GlobalStaticMeshes.get(name);
    }
}