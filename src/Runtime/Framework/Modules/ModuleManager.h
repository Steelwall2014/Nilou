#pragma once

#include "Interface/IRuntimeModule.h"
#include <map>
#include <string>
#include "Templates/TypeTraits.h"

namespace nilou {
    class FModuleManager
    {
    public:

        void AddModule(const std::string &ModuleName, IRuntimeModule *Module);

        IRuntimeModule *GetModule(const std::string &ModuleName);

        std::map<std::string, IRuntimeModule *> Modules;
    };

    FModuleManager *GetModuleManager();


}