#pragma once
#include <string>
#include <map>
#include <memory>
#include "ShaderMap.h"
#include "UniformBuffer.h"


namespace nilou {

    class FContentManager
    {
    
    public:
        FContentManager(const std::filesystem::path &InContentBasePath);

        void Init();

        /**
         * Get the content (e.g. UTexture, UMaterial) by path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The object pointer. User need to cast it manually;
         */
        UObject *GetContentByPath(const std::filesystem::path &InPath);

        /**
         * Create content (e.g. UTexture, UMaterial) at given path
         * @param InPath The path relative to FPath::ContentDir()
         * @param Content The created content
         * 
         */
        bool CreateFile(const std::filesystem::path &InPath, std::unique_ptr<UObject> Content, bool bOverlap=true, bool bNeedFlush=true);

        /**
         * Create directory at given path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         */
        bool CreateDirectory(const std::filesystem::path &InPath, bool bNeedFlush=true);

        void Flush();
    
		void AddGlobalTexture(const std::string &name, std::shared_ptr<class UTexture> texture, bool overlap = false);
		void RemoveGlobalTexture(const std::string &name);
		class UTexture *GetGlobalTexture(const std::string &name);

		void AddGlobalMaterial(const std::string &name, std::shared_ptr<class UMaterial> material, bool overlap = false);
		void RemoveGlobalMaterial(const std::string &name);
		class UMaterial *GetGlobalMaterial(const std::string &name);
    
		void AddGlobalStaticMesh(const std::string &name, std::shared_ptr<class UStaticMesh> mesh, bool overlap = false);
		void RemoveGlobalStaticMesh(const std::string &name);
		class UStaticMesh *GetGlobalStaticMesh(const std::string &name);
    
		void AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> ShaderRHI, bool overlap = false);
		FShaderInstance *GetGlobalShader(const FShaderPermutationParameters &Parameters);
    
		void AddGlobalUniformBuffer(const std::string &name, std::shared_ptr<FUniformBuffer> Buffer, bool overlap = false);
		class FUniformBuffer *GetGlobalUniformBuffer(const std::string &name);

        void ReleaseRenderResources();

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

        class DirectoryEntry
        {
        public:
            bool bIsDirty;
            bool bIsDirectory;
            bool bNeedFlush = true;
            std::string Name;
            /**
             * The absolute path of the directory entry
             */
            std::filesystem::path Path;
            /**
             * The path relative to root entry
             */
            std::filesystem::path RelativePath;
            std::unordered_map<std::string, std::unique_ptr<DirectoryEntry>> Children;
            std::unique_ptr<UObject> Object;

            static std::unique_ptr<DirectoryEntry> Build(const std::filesystem::path &DirectoryPath, const std::filesystem::path &ContentBasePath);

            static UObject *Search(DirectoryEntry *Entry, const std::vector<std::string> &tokens, int depth);

            static void Serialize(DirectoryEntry *Entry);
            static void Deserialize(DirectoryEntry *Entry);
        };
        std::filesystem::path ContentBasePath;
        std::unique_ptr<DirectoryEntry> ContentEntry;

        DirectoryEntry *FContentManager::CreateDirectoryInternal(const std::filesystem::path &InPath, bool bNeedFlush);

        FContentMap<std::string, std::shared_ptr<class UTexture>> GlobalTextures;
        FContentMap<std::string, std::shared_ptr<class UMaterial>> GlobalMaterials;
        FContentMap<std::string, std::shared_ptr<class UStaticMesh>> GlobalStaticMeshes;
        TShaderMap<FShaderPermutationParameters> GlobalShaders;
        std::unordered_map<std::string, std::shared_ptr<FUniformBuffer>> GlobalUniformBuffers;
    };

    FContentManager *GetContentManager();

}