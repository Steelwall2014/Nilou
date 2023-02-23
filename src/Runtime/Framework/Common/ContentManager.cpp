#include <fstream>

#include "ContentManager.h"
#include "Texture.h"
#include "Material.h"
#include "Common/StaticMeshResources.h"

#include "Common/Log.h"
#include "Common/Path.h"
#include "Common/BaseApplication.h"

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

    std::unique_ptr<FContentManager::DirectoryEntry> 
    FContentManager::DirectoryEntry::Build(const fs::path &DirectoryPath, const fs::path &ContentBasePath)
    {
        auto directory_entry = std::make_unique<FContentManager::DirectoryEntry>();
        directory_entry->bIsDirty = false;
        directory_entry->bIsDirectory = true;
        directory_entry->Path = DirectoryPath;
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
                std::ifstream in{Path.generic_string()};
                std::string class_name = ReadClassName(in);
                if (class_name != "")
                {
                    auto file_entry = std::make_unique<FContentManager::DirectoryEntry>();
                    file_entry->bIsDirty = false;
                    file_entry->bIsDirectory = false;
                    file_entry->Path = Path;
                    file_entry->Name = Path.filename().generic_string();
                    file_entry->RelativePath = FPath::RelativePath(ContentBasePath.generic_string(), Path.generic_string());
                    file_entry->Object = FObjectFactory::CreateDefaultObjectByName(class_name);
                    directory_entry->Children[file_entry->Name] = std::move(file_entry);
                }
            }
        }
        return directory_entry;
    }

    UObject *FContentManager::DirectoryEntry::Search(FContentManager::DirectoryEntry *Entry, const std::vector<std::string> &tokens, int depth)
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

    void FContentManager::DirectoryEntry::Serialize(DirectoryEntry *Entry)
    {
        if (Entry->bIsDirectory)
        {
            if (Entry->bIsDirty && Entry->bNeedFlush)
                fs::create_directory(Entry->Path);
            for (auto &[Name, Child] : Entry->Children)
            {
                Serialize(Child.get());
            }
        }
        else 
        {         
            if (Entry->bIsDirty && Entry->bNeedFlush)
            {
                nlohmann::json json;
                std::ofstream out{Entry->Path.generic_string()};
                Entry->Object->Serialize(json, Entry->RelativePath);
                out << json.dump();
            }
        }
    }

    void FContentManager::DirectoryEntry::Deserialize(DirectoryEntry *Entry)
    {
        if (Entry->bIsDirectory)
        {
            for (auto &[Name, Child] : Entry->Children)
            {
                Deserialize(Child.get());
            }
        }
        else 
        {                
            nlohmann::json json;
            std::ifstream in{Entry->Path.generic_string()};
            in >> json;
            if (json.contains("ClassName"))
            {
                if (Entry->Object != nullptr)
                    Entry->Object->Deserialize(json, Entry->RelativePath);
            }
        }
    }

    FContentManager::FContentManager(const fs::path &InContentBasePath)
        : ContentBasePath(InContentBasePath)
    {
        ContentEntry = DirectoryEntry::Build(ContentBasePath, ContentBasePath);
    }

    void FContentManager::Init()
    {
        DirectoryEntry::Deserialize(ContentEntry.get());
    }

    UObject *FContentManager::GetContentByPath(const fs::path &InPath)
    {
        std::string path = InPath.generic_string();
        auto tokens = GameStatics::Split(path, '/');
        tokens.insert(tokens.begin(), "Content");
        return DirectoryEntry::Search(ContentEntry.get(), tokens, 0);
    }

    bool FContentManager::CreateDirectory(const std::filesystem::path &InPath, bool bNeedFlush)
    {
        DirectoryEntry *entry = CreateDirectoryInternal(InPath, bNeedFlush);
        if (entry) 
            return true;
        else
            return false;
    }

    FContentManager::DirectoryEntry *FContentManager::CreateDirectoryInternal(const std::filesystem::path &InPath, bool bNeedFlush)
    {
        std::string path = InPath.generic_string();
        auto tokens = GameStatics::Split(path, '/');
        DirectoryEntry *temp_entry = ContentEntry.get();
        int depth = 0;
        DirectoryEntry *res;
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
                auto Entry = std::make_unique<DirectoryEntry>();
                Entry->Path = temp_entry->Path / fs::path(tokens[depth]);
                Entry->RelativePath = FPath::RelativePath(FPath::ContentDir().generic_string(), Entry->Path.generic_string());
                Entry->Name = tokens[depth];
                Entry->bIsDirectory = true;
                Entry->bIsDirty = true;
                Entry->bNeedFlush = bNeedFlush;
                DirectoryEntry *raw = Entry.get();
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

    bool FContentManager::CreateFile(const std::filesystem::path &InPath, std::unique_ptr<UObject> Content, bool bOverlap, bool bNeedFlush)
    {
        DirectoryEntry *entry = CreateDirectoryInternal(InPath.parent_path(), bNeedFlush);
        if (entry)
        {
            std::string filename = InPath.filename().generic_string();
            if (entry->Children.find(filename) != entry->Children.end())
            {
                if (entry->Children[filename]->bIsDirectory)
                    return false;
                else if (bOverlap)
                {
                    entry->Children[filename]->Object = std::move(Content);
                    entry->Children[filename]->bIsDirty = true;
                    entry->Children[filename]->bNeedFlush = bNeedFlush;
                    return true;
                }
                else 
                {
                    return false;
                }
            }
            else 
            {
                auto Entry = std::make_unique<DirectoryEntry>();
                Entry->Path = entry->Path / InPath.filename();
                Entry->RelativePath = FPath::RelativePath(FPath::ContentDir().generic_string(), Entry->Path.generic_string());
                Entry->Name = InPath.filename().generic_string();
                Entry->bIsDirectory = false;
                Entry->bIsDirty = true;
                Entry->bNeedFlush = bNeedFlush;
                Entry->Object = std::move(Content);
                entry->Children[filename] = std::move(Entry);
                return true;
            }
        }
        return false;
        // std::string path = InPath.generic_string();
        // auto tokens = GameStatics::Split(path, '/');
        // DirectoryEntry *temp_entry = ContentEntry.get();
        // int depth = 0;
        // while (depth < tokens.size())
        // {
        //     if (temp_entry->Children.find(tokens[depth]) != temp_entry->Children.end())
        //     {
        //         temp_entry = temp_entry->Children[tokens[depth]].get();
        //         if (temp_entry->bIsDirectory && depth == tokens.size()-1)
        //             return false;
        //     }
        //     else 
        //     {
        //         auto Entry = std::make_unique<DirectoryEntry>();
        //         Entry->Path = temp_entry->Path / fs::path(tokens[depth]);
        //         Entry->RelativePath = FPath::RelativePath(FPath::ContentDir().generic_string(), Entry->Path.generic_string());
        //         Entry->Name = tokens[depth];
        //         Entry->bIsDirty = true;
        //         Entry->bNeedFlush = bNeedFlush;
        //         if (depth != tokens.size()-1)
        //         {
        //             Entry->bIsDirectory = true;
        //         }
        //         else 
        //         {
        //             Entry->bIsDirectory = false;
        //             Entry->Object = std::move(Content);
        //         }
        //         DirectoryEntry *raw = Entry.get();
        //         temp_entry->Children[Entry->Name] = std::move(Entry);
        //         temp_entry = raw;
        //     }
        //     depth++;
        // }
        return true;
    }

    void FContentManager::Flush()
    {
        DirectoryEntry::Serialize(ContentEntry.get());
    }

    // void FContentManager::AddGlobalTexture(const std::string &name, std::shared_ptr<UTexture> texture, bool overlap)
    // {
    //     if (!GlobalTextures.Insert(name, texture, overlap))
    //         NILOU_LOG(Error, "FContentManager::AddGlobalTexture: Texture \"" + name + "\" already exists, but overlap parameter is set to false");
    // }
    // void FContentManager::RemoveGlobalTexture(const std::string &name)
    // {
    //     GlobalTextures.Erase(name);
    // }
    // UTexture *FContentManager::GetGlobalTexture(const std::string &name)
    // {
    //     UTexture *Texture = GlobalTextures.Get(name).get();
    //     if (Texture == nullptr)
    //         NILOU_LOG(Error, "FContentManager::GetGlobalTexture: Texture \"" + name + "\" doesn't exist");
    //     return Texture;
    // }

    // void FContentManager::AddGlobalMaterial(const std::string &name, std::shared_ptr<UMaterial> material, bool overlap)
    // {
    //     if (!GlobalMaterials.Insert(name, material, overlap))
    //         NILOU_LOG(Error, "FContentManager::AddGlobalMaterial: Material \"" + name + "\" already exists, but overlap parameter is set to false");
    // }
    // void FContentManager::RemoveGlobalMaterial(const std::string &name)
    // {
    //     GlobalMaterials.Erase(name);
    // }
    // UMaterial *FContentManager::GetGlobalMaterial(const std::string &name)
    // {
    //     UMaterial *Material = GlobalMaterials.Get(name).get();
    //     if (Material == nullptr)
    //         NILOU_LOG(Error, "FContentManager::GetGlobalMaterial: Material \"" + name + "\" doesn't exist");
    //     return Material;
    // }

    // void FContentManager::AddGlobalStaticMesh(const std::string &name, std::shared_ptr<UStaticMesh> mesh, bool overlap)
    // {
    //     if (!GlobalStaticMeshes.Insert(name, mesh, overlap))
    //         NILOU_LOG(Error, "FContentManager::AddGlobalStaticMesh: StaticMesh \"" + name + "\" already exists, but overlap parameter is set to false");
    // }
    // void FContentManager::RemoveGlobalStaticMesh(const std::string &name)
    // {
    //     GlobalStaticMeshes.Erase(name);
    // }
    // UStaticMesh *FContentManager::GetGlobalStaticMesh(const std::string &name)
    // {
    //     std::shared_ptr<UStaticMesh> Mesh = GlobalStaticMeshes.Get(name);
    //     if (Mesh == nullptr)
    //         NILOU_LOG(Error, "FContentManager::GetGlobalStaticMesh: StaticMesh \"" + name + "\" doesn't exist");
    //     return Mesh.get();
    // }
    
    void FContentManager::AddGlobalShader(const FShaderPermutationParameters &Parameters, std::shared_ptr<FShaderInstance> ShaderRHI, bool overlap)
    {
        GlobalShaders.AddShader(ShaderRHI, Parameters);
    }

    FShaderInstance *FContentManager::GetGlobalShader(const FShaderPermutationParameters &Parameters)
    {
        return GlobalShaders.GetShader(Parameters);
    }
    
    // void FContentManager::AddGlobalUniformBuffer(const std::string &name, std::shared_ptr<FUniformBuffer> ShaderRHI, bool overlap)
    // {
    //     if (overlap || GlobalUniformBuffers.find(name) == GlobalUniformBuffers.end())
    //         GlobalUniformBuffers[name] = ShaderRHI;
    // }

    // FUniformBuffer *FContentManager::GetGlobalUniformBuffer(const std::string &name)
    // {
    //     return GlobalUniformBuffers[name].get();
    // }


    void FContentManager::ReleaseRenderResources()
    {
        ENQUEUE_RENDER_COMMAND(FContentManager_ReleaseRenderResources)(
            [this](FDynamicRHI*) {
                // GlobalTextures.Map.clear();
                // GlobalMaterials.Map.clear();
                // GlobalStaticMeshes.Map.clear();
                GlobalShaders.RemoveAllShaders();
                // GlobalUniformBuffers.clear();
            });
    }

    FContentManager *GetContentManager()
    {
        return GetAppication()->GetContentManager();
    }
}