#include "Object.h"
#include "Package.h"

namespace nilou {

class FUObjectHashTables
{
public:
    std::mutex CriticalSection;
    std::unordered_map<std::string, NObject*> ObjectMap;

    static FUObjectHashTables& Get()
    {
        static FUObjectHashTables Instance;
        return Instance;
    }
};

NObject* LoadObject(const std::string& Path)
{
    NObject* Object = FindObject(Path);
    if (Object)
        return Object;

    constexpr std::string Delimiter = ".";
    auto LastDot = Path.find_last_of(Delimiter);
    std::string PackageName = Path.substr(0, LastDot);
    std::string ObjectName = Path.substr(LastDot + 1);

    NPackage* Package = LoadPackage(PackageName);
    if (!Package) return nullptr;

    Object = FindObject(Path);
    return Object;
}

NObject* FindObject(const std::string& Path)
{
    std::lock_guard<std::mutex> Lock(FUObjectHashTables::Get().CriticalSection);
    auto& ObjectMap = FUObjectHashTables::Get().ObjectMap;
    auto Found = ObjectMap.find(Path);
    if (Found != ObjectMap.end())
        return Found->second;

    return nullptr;
}

NPackage* LoadPackage(const std::string& Path)
{
    constexpr std::string Delimiter = ".";
    auto LastDot = Path.find_last_of(Delimiter);
    std::string PackageName = Path.substr(0, LastDot);

    NObject* Object = FindObject(PackageName);
    if (Object) return Cast<NPackage>(Object);

    return nullptr;
}

}