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
         * @return The object pointer. User need to cast it manually.
         */
        UObject *GetContentByPath(const std::filesystem::path &InPath);

        /**
         * Get the material by path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The material pointer. 
         */
        class UMaterial *GetMaterialByPath(const std::filesystem::path &InPath);

        /**
         * Get the texture by path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The texture pointer. 
         */
        class UTexture *GetTextureByPath(const std::filesystem::path &InPath);

        /**
         * Get the staic mesh by path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The staic mesh pointer. 
         */
        class UStaticMesh *GetStaticMeshByPath(const std::filesystem::path &InPath);

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
    
		void AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> ShaderRHI, bool overlap = false);
		FShaderInstance *GetGlobalShader(const FShaderPermutationParameters &Parameters);

        void ReleaseRenderResources();

    private:

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
            static void Deserialize(DirectoryEntry *Entry, std::vector<FContentManager::DirectoryEntry*> &OutEntries);
        };
        std::filesystem::path ContentBasePath;
        std::unique_ptr<DirectoryEntry> ContentEntry;

        DirectoryEntry *FContentManager::CreateDirectoryInternal(const std::filesystem::path &InPath, bool bNeedFlush);

        TShaderMap<FShaderPermutationParameters> GlobalShaders;
    };

    FContentManager *GetContentManager();

}