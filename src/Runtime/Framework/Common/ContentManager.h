#pragma once
#include <string>
#include <map>
#include <memory>
#include "ShaderMap.h"


namespace nilou {

    class FContentManager
    {
    
    public:
        FContentManager()
        {

        }
    
		void AddGlobalTexture(const std::string &name, std::shared_ptr<class FTexture> texture, bool overlap = false);
		void RemoveGlobalTexture(const std::string &name);
		class FTexture *GetGlobalTexture(const std::string &name);

		void AddGlobalMaterial(const std::string &name, std::shared_ptr<class FMaterial> material, bool overlap = false);
		void RemoveGlobalMaterial(const std::string &name);
		class FMaterial *GetGlobalMaterial(const std::string &name);
    
		void AddGlobalStaticMesh(const std::string &name, std::shared_ptr<class UStaticMesh> mesh, bool overlap = false);
		void RemoveGlobalStaticMesh(const std::string &name);
		class UStaticMesh *GetGlobalStaticMesh(const std::string &name);
    
		void AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> mesh, bool overlap = false);
		FShaderInstance *GetGlobalShader(const FShaderPermutationParameters &Parameters);

        void ReleaseRenderResources();

        static FContentManager &GetContentManager();

    private:

        template<class Key, class Value>
        struct FContentMap
        {
            std::map<Key, Value> Map;
            bool Insert(const Key &key, const Value &value, bool overlap=false)
            {
                auto iter = Map.find(key);
                if (iter == Map.end() || overlap)
                {
                    Map[key] = value;
                }
                else 
                {
                    return false;
                }
                return true;
            }
            Value Get(const Key &key)
            {
                auto iter = Map.find(key);
                if (iter == Map.end())
                    return nullptr;
                else
                    return iter->second;
            }
            void Erase(const Key &key)
            {
                auto iter = Map.find(key);
                if (iter != Map.end())
                    Map.erase(iter);
            }
        };

        FContentMap<std::string, std::shared_ptr<class FTexture>> GlobalTextures;
        FContentMap<std::string, std::shared_ptr<class FMaterial>> GlobalMaterials;
        FContentMap<std::string, std::shared_ptr<class UStaticMesh>> GlobalStaticMeshes;
        TShaderMap<FShaderPermutationParameters> GlobalShaders;
    };

}