#pragma once
#include <string>
#include <map>
#include <memory>
#include "ShaderMap.h"
#include "UniformBuffer.h"
#include "Common/Path.h"


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
         * @brief Create a defualt content (e.g. UTexture, UMaterial) at given path
         * The path shouldn't be occupied by other content/directory sharing the same name.
         * 
         * @tparam The desired content class
         * 
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The pointer to the created object, nullptr if target path is occupied.
         * 
         */
        template<typename T>
        T *CreateFile(const std::filesystem::path &InPath, bool bNeedFlush=true)
        {
            DirectoryEntry *entry = CreateDirectoryInternal(InPath.parent_path(), bNeedFlush);
            if (entry)
            {
                std::string filename = InPath.filename().generic_string();
                if (entry->Children.find(filename) == entry->Children.end())
                {
                    auto Entry = std::make_unique<DirectoryEntry>();
                    Entry->AbsolutePath = entry->AbsolutePath / InPath.filename();
                    Entry->RelativePath = FPath::RelativePath(FPath::ContentDir().generic_string(), Entry->AbsolutePath.generic_string());
                    Entry->Name = InPath.filename().generic_string();
                    Entry->bIsDirectory = false;
                    Entry->bIsDirty = true;
                    Entry->bNeedFlush = bNeedFlush;
                    Entry->Object = std::make_unique<T>();
                    T *raw_p = Entry->Object.get();
                    entry->Children[filename] = std::move(Entry);
                    return raw_p;
                }
            }
            return nullptr;
        }

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
            std::filesystem::path AbsolutePath;
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