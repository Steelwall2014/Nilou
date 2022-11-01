#include "TextureManager.h"

namespace und {
    TextureManager *g_pTextureManager = new TextureManager;

    void TextureManager::AddGlobalTexture(const std::string &name, RHITextureRef texture, bool overlap)
    {
        if (overlap)
        {
            GlobalTextures[name] = texture;
            return;
        }
        if (GlobalTextures.find(name) != GlobalTextures.end())
            throw("Texture: " + name + " exist");
        GlobalTextures[name] = texture;
    }
    void TextureManager::RemoveGlobalTexture(const std::string &name)
    {
        GlobalTextures.erase(name);
    }
    RHITextureRef TextureManager::GetGlobalTexture(const std::string &name)
    {
        if (GlobalTextures.find(name) == GlobalTextures.end())
            return nullptr;
        return GlobalTextures[name];
    }
}
