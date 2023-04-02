#include <fstream>

#include <thread_pool/BS_thread_pool.hpp>

#include "ContentManager.h"
#include "Texture.h"
#include "Material.h"
#include "StaticMeshResources.h"

#include "Common/Log.h"
#include "BaseApplication.h"

namespace nilou {

    namespace fs = std::filesystem;

    static std::string ReadClassName(std::ifstream &in)
    {
        char c;
        int i = 0;
        std::string temp;
        std::string res;
        while (i < 3 && !in.eof())
        {
            in.read(&c, 1);
            if (c == ' ' || c == '\n' || c == '\t')
                continue;
            if (c == '\"')
                i++;
            temp += c;
        }
        if (temp == "{\"ClassName\":\"")
        {
            while (true)
            {
                in.read(&c, 1);
                if (c == '\"')
                    break;
                else
                    res += c;
            }
        }
        return res;
    }

    std::unique_ptr<FContentEntry> 
    FContentEntry::Build(const fs::path &DirectoryPath, const fs::path &ContentBasePath)
    {
        auto directory_entry = std::make_unique<FContentEntry>();
        directory_entry->bIsDirty = false;
        directory_entry->bIsDirectory = true;
        directory_entry->AbsolutePath = DirectoryPath;
        directory_entry->Name = DirectoryPath.filename().generic_string();
        directory_entry->RelativePath = FPath::RelativePath(ContentBasePath.generic_string(), DirectoryPath.generic_string());
        for (const fs::directory_entry &dir_entry : fs::directory_iterator(DirectoryPath))
        {
            if (dir_entry.is_directory())
            {
                auto entry = Build(dir_entry.path(), ContentBasePath);
                directory_entry->Children[entry->Name] = std::move(entry);
            }
            else 
            {
                fs::path Path = dir_entry.path();
                if (Path.extension().generic_string() != ".nasset")
                    continue;
                std::ifstream in{Path.generic_string()};
                // std::string class_name = ReadClassName(in);
                // if (class_name != "")
                // {
                auto file_entry = std::make_unique<FContentEntry>();
                file_entry->bIsDirty = false;
                file_entry->bIsDirectory = false;
                file_entry->AbsolutePath = Path;
                file_entry->Name = Path.filename().generic_string();
                file_entry->RelativePath = FPath::RelativePath(ContentBasePath.generic_string(), Path.generic_string());
                // file_entry->Object = FObjectFactory::CreateDefaultObjectByName(class_name);
                // file_entry->Object->SerializationPath = file_entry->RelativePath;
                directory_entry->Children[file_entry->Name] = std::move(file_entry);
                // }
            }
        }
        return directory_entry;
    }

    UObject *FContentEntry::Search(FContentEntry *Entry, const std::vector<std::string> &tokens, int depth)
    {
        if (tokens[depth] == Entry->Name)
        {
            if (Entry->bIsDirectory)
            {
                for (auto &[Name, Child] : Entry->Children)
                {
                    UObject *object = Search(Child.get(), tokens, depth+1);
                    if (object != nullptr)
                        return object;
                }
            }
            else
            {
                return Entry->Object.get();
            }
        }

        return nullptr;
    }

    void FContentEntry::Serialize(FContentEntry *Entry)
    {
        if (Entry->bIsDirectory)
        {
            if (Entry->bIsDirty && Entry->bNeedFlush)
                fs::create_directory(Entry->AbsolutePath);
            for (auto &[Name, Child] : Entry->Children)
            {
                Serialize(Child.get());
            }
        }
        else 
        {         
            if (Entry->bIsDirty && Entry->bNeedFlush && !Entry->Object->SerializationPath.empty())
            {
                FArchive Ar;
                Entry->Object->Serialize(Ar);
                Ar.WriteToPath(Entry->AbsolutePath);
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
        ContentEntry = FContentEntry::Build(ContentBasePath, ContentBasePath);
    }

    void FContentManager::Init()
    {
        std::vector<FContentEntry*> Entries;
        FContentEntry::Deserialize(ContentEntry.get(), Entries);
        BS::thread_pool pool;
        std::vector<std::future<FArchive>> futures;
        for (int i = 0; i < Entries.size(); i++)
        {
            auto future = pool.submit([](FContentEntry *Entry) {
                FArchive Ar;
                std::filesystem::path InPath = Entry->AbsolutePath;
                InPath.replace_extension(".nasset");
                Ar.LoadFromPath(InPath);
                return Ar;
            }, Entries[i]);
            futures.push_back(std::move(future));
        }
        pool.wait_for_tasks();
        std::vector<FArchive> Archives;
        for (int i = 0; i < futures.size(); i++)
        {
            auto &future = futures[i];
            FArchive Ar = future.get();
            if (Ar.json.contains("ClassName"))
            {
                Entries[i]->Object = FObjectFactory::CreateDefaultObjectByName(Ar.json["ClassName"]);
                Entries[i]->Object->SerializationPath = Entries[i]->RelativePath;
                Entries[i]->Object->ContentEntry = Entries[i];
            }
            Archives.push_back(std::move(Ar));
        }
        for (int i = 0; i < Archives.size(); i++)
        {
            Entries[i]->Object->Deserialize(Archives[i]);
        }
    }

    UObject *FContentManager::GetContentByPath(const fs::path &InPath)
    {
        std::string path = InPath.generic_string();
        auto tokens = GameStatics::Split(path, '/');
        tokens.insert(tokens.begin(), "Content");
        return FContentEntry::Search(ContentEntry.get(), tokens, 0);
    }

    UMaterial *FContentManager::GetMaterialByPath(const fs::path &InPath)
    {
        return dynamic_cast<UMaterial*>(GetContentByPath(InPath));
    }

    UTexture *FContentManager::GetTextureByPath(const fs::path &InPath)
    {
        return dynamic_cast<UTexture*>(GetContentByPath(InPath));
    }

    UStaticMesh *FContentManager::GetStaticMeshByPath(const fs::path &InPath)
    {
        return dynamic_cast<UStaticMesh*>(GetContentByPath(InPath));
    }

    bool FContentManager::CreateDirectory(const std::filesystem::path &InPath, bool bNeedFlush)
    {
        FContentEntry *entry = CreateDirectoryInternal(InPath, bNeedFlush);
        if (entry) 
            return true;
        else
            return false;
    }

    FContentEntry *FContentManager::CreateDirectoryInternal(const std::filesystem::path &InPath, bool bNeedFlush)
    {
        std::string path = InPath.generic_string();
        auto tokens = GameStatics::Split(path, '/');
        FContentEntry *temp_entry = ContentEntry.get();
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
                Entry->RelativePath = FPath::RelativePath(FPath::ContentDir().generic_string(), Entry->AbsolutePath.generic_string());
                Entry->Name = tokens[depth];
                Entry->bIsDirectory = true;
                Entry->bIsDirty = true;
                Entry->bNeedFlush = bNeedFlush;
                FContentEntry *raw = Entry.get();
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
        FContentEntry::Serialize(ContentEntry.get());
    }
    
    void FContentManager::AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> ShaderRHI, bool overlap)
    {
        GlobalShaders.AddShader(ShaderRHI, Parameters);
    }

    FShaderInstance *FContentManager::GetGlobalShader(const FShaderPermutationParameters &Parameters)
    {
        return GlobalShaders.GetShader(Parameters);
    }

    void FContentManager::ReleaseRenderResources()
    {
        ENQUEUE_RENDER_COMMAND(FContentManager_ReleaseRenderResources)(
            [this](FDynamicRHI*) {
                GlobalShaders.RemoveAllShaders();
                ForEachContent(
                    [](UObject* Obj) {
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

    void FContentManager::ForEachContent(std::function<void(UObject*)> &&Func)
    {
        ForEachContentInternal(ContentEntry.get(), std::forward<std::function<void(UObject*)>>(Func));
    }

    void FContentManager::ForEachEntry(std::function<void(FContentEntry*)> &&Func)
    {
        ForEachEntryInternal(ContentEntry.get(), std::forward<std::function<void(FContentEntry*)>>(Func));
    }

    void FContentManager::ForEachContentInternal(FContentEntry* Entry, std::function<void(UObject*)> &&Func)
    {
        if (Entry->bIsDirectory)
        {
            for (auto &[Name, Child] : Entry->Children)
            {
                ForEachContentInternal(Child.get(), std::forward<std::function<void(UObject*)>>(Func));
            }
        }
        else 
        {              
            Func(Entry->Object.get());
        }
    }

    void FContentManager::ForEachEntryInternal(FContentEntry* Entry, std::function<void(FContentEntry*)> &&Func)
    {
        if (Entry->bIsDirectory)
        {
            for (auto &[Name, Child] : Entry->Children)
            {
                ForEachEntryInternal(Child.get(), std::forward<std::function<void(FContentEntry*)>>(Func));
            }
        }
        else 
        {              
            Func(Entry);
        }
    }
}