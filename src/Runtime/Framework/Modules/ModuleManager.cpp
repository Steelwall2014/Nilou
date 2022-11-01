#include "ModuleManager.h"
#include "Interface/IRuntimeModule.h"

namespace nilou {

    FModuleManager *GetModuleManager()
    {
        static FModuleManager GModuleManager;
        return &GModuleManager;
    }

    void FModuleManager::AddModule(const std::string &ModuleName, IRuntimeModule *Module)
    {
        Modules[ModuleName] = Module;
    }
    
    IRuntimeModule *FModuleManager::GetModule(const std::string &ModuleName)
    {
        return Modules[ModuleName];
    }


}