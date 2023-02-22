#include "ContentManager.h"
#include "Texture.h"
#include "Material.h"
#include "Common/StaticMeshResources.h"

#include "Common/Log.h"

namespace nilou {


    FContentManager &FContentManager::GetContentManager()
    {
        static FContentManager GContentManager;
        return GContentManager;
    }

    void FContentManager::AddGlobalTexture(const std::string &name, std::shared_ptr<UTexture> texture, bool overlap)
    {
        if (!GlobalTextures.Insert(name, texture, overlap))
            NILOU_LOG(Error, "FContentManager::AddGlobalTexture: Texture \"" + name + "\" already exists, but overlap parameter is set to false");
    }
    void FContentManager::RemoveGlobalTexture(const std::string &name)
    {
        GlobalTextures.Erase(name);
    }
    UTexture *FContentManager::GetGlobalTexture(const std::string &name)
    {
        UTexture *Texture = GlobalTextures.Get(name).get();
        if (Texture == nullptr)
            NILOU_LOG(Error, "FContentManager::GetGlobalTexture: Texture \"" + name + "\" doesn't exist");
        return Texture;
    }

    void FContentManager::AddGlobalMaterial(const std::string &name, std::shared_ptr<UMaterial> material, bool overlap)
    {
        if (!GlobalMaterials.Insert(name, material, overlap))
            NILOU_LOG(Error, "FContentManager::AddGlobalMaterial: Material \"" + name + "\" already exists, but overlap parameter is set to false");
    }
    void FContentManager::RemoveGlobalMaterial(const std::string &name)
    {
        GlobalMaterials.Erase(name);
    }
    UMaterial *FContentManager::GetGlobalMaterial(const std::string &name)
    {
        UMaterial *Material = GlobalMaterials.Get(name).get();
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
    UStaticMesh *FContentManager::GetGlobalStaticMesh(const std::string &name)
    {
        std::shared_ptr<UStaticMesh> Mesh = GlobalStaticMeshes.Get(name);
        if (Mesh == nullptr)
            NILOU_LOG(Error, "FContentManager::GetGlobalStaticMesh: StaticMesh \"" + name + "\" doesn't exist");
        return Mesh.get();
    }
    
    void FContentManager::AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> ShaderRHI, bool overlap)
    {
        GlobalShaders.AddShader(ShaderRHI, Parameters);
    }

    FShaderInstance *FContentManager::GetGlobalShader(const FShaderPermutationParameters &Parameters)
    {
        return GlobalShaders.GetShader(Parameters);
    }
    
    void FContentManager::AddGlobalUniformBuffer(const std::string &name, std::shared_ptr<UUniformBuffer> ShaderRHI, bool overlap)
    {
        if (overlap || GlobalUniformBuffers.find(name) == GlobalUniformBuffers.end())
            GlobalUniformBuffers[name] = ShaderRHI;
    }

    UUniformBuffer *FContentManager::GetGlobalUniformBuffer(const std::string &name)
    {
        return GlobalUniformBuffers[name].get();
    }


    void FContentManager::ReleaseRenderResources()
    {
        ENQUEUE_RENDER_COMMAND(FContentManager_ReleaseRenderResources)(
            [this](FDynamicRHI*) {
                GlobalTextures.Map.clear();
                GlobalMaterials.Map.clear();
                GlobalStaticMeshes.Map.clear();
                GlobalShaders.RemoveAllShaders();
                GlobalUniformBuffers.clear();
            });
    }
}