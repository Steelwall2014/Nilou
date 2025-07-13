#include <fstream>

#include <thread_pool/BS_thread_pool.hpp>

#include "ContentManager.h"
#include "Texture.h"
#include "Material.h"
#include "StaticMeshResources.h"

#include "Common/Log.h"
#include "BaseApplication.h"

namespace nilou {

	TShaderMap<FShaderPermutationParameters> GlobalShaders;
    void AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> ShaderRHI, bool overlap)
    {
        GlobalShaders.AddShader(ShaderRHI, Parameters);
    }

    FShaderInstance *GetGlobalShader(const FShaderPermutationParameters &Parameters)
    {
        return GlobalShaders.GetShader(Parameters);
    }

    struct FArchiveHelper
    {
        nlohmann::json root;
        std::vector<FArchiveBuffer> buffers;
        FArchive Ar;
        FArchiveHelper()
            : Ar(root, buffers)
        { }
    };

    namespace fs = std::filesystem;

    std::unique_ptr<FContentEntry> 
    FContentEntry::Build(const fs::path &DirectoryPath, const fs::path &ContentBasePath)
    {
        auto directory_entry = std::make_unique<FContentEntry>();
        directory_entry->bIsDirty = false;
        directory_entry->bIsDirectory = true;
        directory_entry->AbsolutePath = DirectoryPath;
        directory_entry->Name = DirectoryPath.filename().generic_string();
        directory_entry->VirtualPath = "/" + FPath::RelativePath(ContentBasePath.generic_string(), DirectoryPath.generic_string());
        for (const fs::directory_entry &dir_entry : fs::directory_iterator(DirectoryPath))
        {
            if (dir_entry.is_directory())
            {
                auto entry = Build(dir_entry.path(), ContentBasePath);
                entry->Parent = directory_entry.get();
                directory_entry->Children[entry->Name] = std::move(entry);
            }
            else 
            {
                fs::path Path = dir_entry.path();
                if (Path.extension().generic_string() != ".nasset")
                    continue;
                std::ifstream in{Path.generic_string()};
                auto file_entry = std::make_unique<FContentEntry>();
                file_entry->bIsDirty = false;
                file_entry->bIsDirectory = false;
                file_entry->AbsolutePath = Path;
                file_entry->Name = Path.filename().generic_string();
                file_entry->VirtualPath = "/" + FPath::RelativePath(ContentBasePath.generic_string(), Path.generic_string());
                file_entry->Parent = directory_entry.get();
                directory_entry->Children[file_entry->Name] = std::move(file_entry);
            }
        }
        return directory_entry;
    }

    FContentEntry* FContentEntry::Search(FContentEntry *Entry, const std::vector<std::string> &tokens, int depth)
    {
        if (tokens[depth] == Entry->Name)
        {
            if (Entry->bIsDirectory)
            {
                for (auto &[Name, Child] : Entry->Children)
                {
                    FContentEntry *entry = Search(Child.get(), tokens, depth+1);
                    if (entry != nullptr)
                        return entry;
                }
            }
            else
            {
                return Entry;
            }
        }

        return nullptr;
    }

    void FContentEntry::Serialize(FContentEntry *Entry, bool bRecursive)
    {
        if (Entry->bIsDirectory)
        {
            if (Entry->bIsDirty)
                fs::create_directory(Entry->AbsolutePath);
            if (bRecursive)
            {
                for (auto &[Name, Child] : Entry->Children)
                {
                    Serialize(Child.get(), bRecursive);
                }
            }
        }
        else 
        {         
            if (Entry->bIsDirty)
            {
                FArchiveHelper ArHelper;
                Entry->Object->PreSerialize(ArHelper.Ar);
                Entry->Object->Serialize(ArHelper.Ar);
                Entry->Object->PostSerialize(ArHelper.Ar);
                std::ofstream out(Entry->AbsolutePath, std::ios::binary);
                out << ArHelper.Ar;
                Entry->bIsDirty = false;
            }
        }
    }

    void FContentEntry::Deserialize(FContentEntry *Entry, std::vector<FContentEntry*> &OutEntries)
    {
        if (Entry->bIsDirectory)
        {
            for (auto &[Name, Child] : Entry->Children)
            {
                Deserialize(Child.get(), OutEntries);
            }
        }
        else 
        {              
            OutEntries.push_back(Entry);
        }
    }

    FContentManager::FContentManager(const fs::path &InContentBasePath)
        : ContentBasePath(InContentBasePath)
    {
        RootEntry = FContentEntry::Build(ContentBasePath, ContentBasePath);
    }

    void FContentManager::Init()
    {
        std::vector<FContentEntry*> Entries;
        FContentEntry::Deserialize(RootEntry.get(), Entries);
        BS::thread_pool pool;
        std::vector<std::future<std::unique_ptr<FArchiveHelper>>> futures;
        for (int i = 0; i < Entries.size(); i++)
        {
            auto future = pool.submit([](FContentEntry *Entry) {
                std::unique_ptr<FArchiveHelper> ArHelper = std::make_unique<FArchiveHelper>();
                std::filesystem::path InPath = Entry->AbsolutePath;
                InPath.replace_extension(".nasset");
                std::ifstream in(InPath, std::ios::binary);
                in >> ArHelper->Ar;
                return ArHelper;
            }, Entries[i]);
            futures.push_back(std::move(future));
        }
        pool.wait_for_tasks();
        std::vector<std::unique_ptr<FArchiveHelper>> Archives;
        for (int i = 0; i < futures.size(); i++)
        {
            auto &future = futures[i];
            std::unique_ptr<FArchiveHelper> ArHelper = future.get();
            if (ArHelper->Ar.Node.contains("ClassName"))
            {
                auto class_name = std::string(ArHelper->Ar.Node["ClassName"]);
                if (!GameStatics::StartsWith(class_name, "nilou::"))
                    class_name = "nilou::" + class_name;
                NAsset* asset = static_cast<NAsset*>(CreateDefaultObject(class_name));
                if (asset == nullptr)
                {
                    NILOU_LOG(Warning, "Failed to create object of class {}", class_name);
                    continue;
                }
                Entries[i]->Object = std::unique_ptr<NAsset>(asset);
                Entries[i]->Object->ContentEntry = Entries[i];
                Archives.push_back(std::move(ArHelper));
            }
        }
        // Because of some dependencies between assets, we need to deserialize them in three steps
        // TODO: dependency graph
        for (int i = 0; i < Archives.size(); i++)
        {
            if (Entries[i]->Object)
            {
                Entries[i]->Object->PreDeserialize(Archives[i]->Ar);
            }
        }
        for (int i = 0; i < Archives.size(); i++)
        {
            if (Entries[i]->Object)
            {
                Entries[i]->Object->Deserialize(Archives[i]->Ar);
            }
        }
        for (int i = 0; i < Archives.size(); i++)
        {
            if (Entries[i]->Object)
            {
                Entries[i]->Object->PostDeserialize(Archives[i]->Ar);
            }
        }
    }

    NAsset *FContentManager::GetContentByPath(const std::string &InPath)
    {
        FContentEntry* Entry = GetEntryByPath(InPath);
        if (Entry)
            return Entry->Object.get();
        else
            return nullptr;
    }

    UMaterial *FContentManager::GetMaterialByPath(const std::string &InPath)
    {
        return dynamic_cast<UMaterial*>(GetContentByPath(InPath));
    }

    UTexture *FContentManager::GetTextureByPath(const std::string &InPath)
    {
        return dynamic_cast<UTexture*>(GetContentByPath(InPath));
    }

    UStaticMesh *FContentManager::GetStaticMeshByPath(const std::string &InPath)
    {
        return dynamic_cast<UStaticMesh*>(GetContentByPath(InPath));
    }

    NAsset* FContentManager::CreateAsset(const std::string& Name, const std::string &VirtualDirectory, const NClass* Class)
    {
        if (!Class->IsChildOf(NAsset::StaticClass()))
            return nullptr;
        std::filesystem::path Directory = std::filesystem::path(VirtualDirectory);
        std::filesystem::path InPath = Directory / (Name + ".nasset");
        FContentEntry *temp_entry = CreateDirectoryInternal(Directory);
        if (temp_entry)
        {
            std::string filename = InPath.filename().generic_string();
            if (temp_entry->Children.find(filename) == temp_entry->Children.end())
            {
                auto Entry = std::make_unique<FContentEntry>();
                Entry->AbsolutePath = temp_entry->AbsolutePath / InPath.filename();
                Entry->VirtualPath = "/" + FPath::RelativePath(FPath::ContentDir().generic_string(), Entry->AbsolutePath.generic_string());
                Entry->Name = InPath.filename().generic_string();
                Entry->bIsDirectory = false;
                Entry->bIsDirty = true;
                auto Object = std::unique_ptr<NAsset>(static_cast<NAsset*>(CreateDefaultObject(Class)));
                Object->NamePrivate = Name;
                Object->ContentEntry = Entry.get();
                NAsset *raw_p = Object.get();
                Entry->Object = std::move(Object);
                Entry->Parent = temp_entry;
                temp_entry->Children[filename] = std::move(Entry);
                return raw_p;
            }
        }
        return nullptr;
    }

    bool FContentManager::RenameAsset(const std::string &AssetPathToRename, const std::string &NewName)
    {
        NAsset* AssetToRename = GetContentByPath(AssetPathToRename);
        return RenameAsset(AssetToRename, NewName);
    }

    bool FContentManager::RenameAsset(NAsset* AssetToRename, const std::string &NewName)
    {
        // TODO: search for dependencies
        if (AssetToRename && AssetToRename->ContentEntry)
        {
            FContentEntry *EntryToRename = AssetToRename->ContentEntry;
            fs::path old_absolute_path = EntryToRename->AbsolutePath;
            fs::path virtual_path = fs::path(EntryToRename->VirtualPath).replace_filename(NewName);
            EntryToRename->VirtualPath = virtual_path.generic_string();
            EntryToRename->AbsolutePath = FPath::VirtualPathToAbsPath(EntryToRename->VirtualPath);
            EntryToRename->Name = NewName;
            AssetToRename->NamePrivate = NewName;
            SaveAsset(EntryToRename->VirtualPath);
            fs::remove(old_absolute_path);
            return true;
        }
        return false;
    }

    bool FContentManager::SaveAsset(const std::string &AssetPathToSave)
    {
        NAsset* AssetToSave = GetContentByPath(AssetPathToSave);
        return SaveAsset(AssetToSave);
    }

    bool FContentManager::SaveAsset(NAsset* AssetToSave)
    {
        if (AssetToSave && AssetToSave->ContentEntry)
        {
            FContentEntry::Serialize(AssetToSave->ContentEntry);
            return true;
        }
        return false;
    }

    bool FContentManager::CreateDirectory(const std::filesystem::path &InPath)
    {
        FContentEntry *entry = CreateDirectoryInternal(InPath);
        if (entry) 
            return true;
        else
            return false;
    }

    FContentEntry *FContentManager::CreateDirectoryInternal(const std::filesystem::path &InPath)
    {
        std::string path = InPath.generic_string();
        auto tokens = GameStatics::Split(path, '/');
        FContentEntry *temp_entry = RootEntry.get();
        int depth = 0;
        FContentEntry *res;
        while (depth < tokens.size())
        {
            if (temp_entry->Children.find(tokens[depth]) != temp_entry->Children.end())
            {
                if (temp_entry->bIsDirectory)
                    temp_entry = temp_entry->Children[tokens[depth]].get();
                else
                    return nullptr;
            }
            else 
            {
                auto Entry = std::make_unique<FContentEntry>();
                Entry->AbsolutePath = temp_entry->AbsolutePath / fs::path(tokens[depth]);
                Entry->VirtualPath = "/" + FPath::RelativePath(FPath::ContentDir().generic_string(), Entry->AbsolutePath.generic_string());
                Entry->Name = tokens[depth];
                Entry->bIsDirectory = true;
                Entry->bIsDirty = true;
                FContentEntry *raw = Entry.get();
                Entry->Parent = temp_entry;
                temp_entry->Children[Entry->Name] = std::move(Entry);
                temp_entry = raw;
            }
            depth++;
        }
        if (temp_entry->bIsDirectory)
            return temp_entry;
        else
            return nullptr;
    }

    void FContentManager::Flush()
    {
        FContentEntry::Serialize(RootEntry.get(), true);
    }

    void FContentManager::ReleaseRenderResources()
    {
        ENQUEUE_RENDER_COMMAND(FContentManager_ReleaseRenderResources)(
            [this](RenderGraph&) {
                GlobalShaders.RemoveAllShaders();
                ForEachContent(
                    [](NAsset* Obj) {
                        if (!Obj) return;
                        if (Obj->IsA(UStaticMesh::StaticClass()))
                        {
                            UStaticMesh* mesh = static_cast<UStaticMesh*>(Obj);
                            mesh->ReleaseResources();
                        }
                        else if (Obj->IsA(UTexture::StaticClass())) 
                        {
                            UTexture* texture = static_cast<UTexture*>(Obj);
                            texture->ReleaseResource();
                        }
                        else if (Obj->IsA(UMaterial::StaticClass())) 
                        {
                            UMaterial* material = static_cast<UMaterial*>(Obj);
                            material->ReleaseResources();
                        }
                    });
            });
    }

    FContentManager *GetContentManager()
    {
        return GetAppication()->GetContentManager();
    }

    template<typename T>
    static void ForEachTemplate(FContentEntry* Entry, const std::function<void(T*)> &Func)
    {
        if (Entry->bIsDirectory)
        {
            for (auto &[Name, Child] : Entry->Children)
            {
                ForEachTemplate(Child.get(), Func);
            }
        }
        else 
        {     
            if constexpr (std::is_same_v<T, FContentEntry>)         
                Func(Entry);
            else if constexpr (std::is_same_v<T, NAsset>)
                Func(Entry->Object.get());
        }
    }

    void FContentManager::ForEachContent(const std::function<void(NAsset*)> &Func)
    {
        ForEachTemplate(RootEntry.get(), Func);
    }

    void FContentManager::ForEachEntry(const std::function<void(FContentEntry*)> &Func)
    {
        ForEachTemplate(RootEntry.get(), Func);
    }

    FContentEntry* FContentManager::GetEntryByPath(const std::string &InPath)
    {
        auto tokens = GameStatics::Split(InPath, '/');
        tokens.insert(tokens.begin(), "Content");
        return FContentEntry::Search(RootEntry.get(), tokens, 0);
    }
}