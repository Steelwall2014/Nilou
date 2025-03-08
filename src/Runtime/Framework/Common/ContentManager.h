#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <functional>
// #include "ShaderMap.h"
// #include "UniformBuffer.h"
#include "Common/CoreUObject/Class.h"
#include "Common/Path.h"
#include "Templates/TypeTraits.h"


namespace nilou {

    class FContentEntry
    {
    public:
        friend class FContentManager;
        bool bIsDirty = false;
        bool bIsDirectory;
        std::string Name;
        /**
         * The absolute path of the directory entry
         */
        std::filesystem::path AbsolutePath;
        /**
         * The path relative to root entry but starts with '/'
         * TODO: Use std::string to represent all virtual paths and std::filesystem::path to represent all absolute paths
         */
        std::string VirtualPath;
        FContentEntry* Parent = nullptr;
        std::unordered_map<std::string, std::unique_ptr<FContentEntry>> Children;
        std::unique_ptr<class NAsset> Object;

    private:
        static std::unique_ptr<FContentEntry> Build(const std::filesystem::path &DirectoryPath, const std::filesystem::path &ContentBasePath);
        static FContentEntry *Search(FContentEntry *Entry, const std::vector<std::string> &tokens, int depth);
        static void Serialize(FContentEntry *Entry, bool bRecursive=false);
        static void Deserialize(FContentEntry *Entry, std::vector<FContentEntry*> &OutEntries);
    };

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
        NAsset *GetContentByPath(const std::string &InPath);

        /**
         * Get the material by path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The material pointer. 
         */
        class UMaterial *GetMaterialByPath(const std::string &InPath);

        /**
         * Get the texture by path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The texture pointer. 
         */
        class UTexture *GetTextureByPath(const std::string &InPath);

        /**
         * Get the staic mesh by path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         * @return The staic mesh pointer. 
         */
        class UStaticMesh *GetStaticMeshByPath(const std::string &InPath);

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
        T *CreateAsset(const std::string& Name, const std::string &VirtualDirectory)
        {
            return static_cast<T*>(CreateAsset(Name, VirtualDirectory, T::StaticClass()));
        }

        NAsset* CreateAsset(const std::string& Name, const std::string &VirtualDirectory, const NClass* Class);

        bool RenameAsset(const std::string &AssetPathToRename, const std::string &NewName);
        bool RenameAsset(NAsset* AssetToRename, const std::string &NewName);

        bool SaveAsset(const std::string &AssetPathToSave);
        bool SaveAsset(NAsset* AssetToSave);

        /**
         * Create directory at given path
         * @param InPath The path relative to FPath::ContentDir()
         * 
         */
        bool CreateDirectory(const std::filesystem::path &InPath);

        void Flush();

        void ReleaseRenderResources();

        void ForEachContent(const std::function<void(NAsset*)> &Func);

        void ForEachEntry(const std::function<void(FContentEntry*)> &Func);

    private:


        std::filesystem::path ContentBasePath;
        std::unique_ptr<FContentEntry> RootEntry;

        FContentEntry *CreateDirectoryInternal(const std::filesystem::path &InPath);

        FContentEntry* GetEntryByPath(const std::string &InPath);
    };

    FContentManager *GetContentManager();

}