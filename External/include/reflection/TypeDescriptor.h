#pragma once
#include <string>
#include <map>
#include <set>
#include <memory>
#include <queue>

#include "MemberVariable.h"
#include "MemberFunction.h"

namespace reflection {

    class TypeDescriptor
    {
    public:
        friend class RawTypeDescriptorBuilder;

        const std::string& GetTypeName() const
        {
            return TypeName;
        }

        const MemberVariable* GetMemberVariable(const std::string& VariableName) const
        {
            if (MemberVariables.contains(VariableName))
                return MemberVariables.at(VariableName).get();
            return nullptr;
        }

        const MemberFunction* GetMemberFunction(const std::string& FunctionName) const
        {
            if (MemberFunctions.contains(FunctionName))
                return MemberFunctions.at(FunctionName).get();
            return nullptr;
        }

        const Constructor& GetDefaultConstructor() const
        {
            return *DefaultCtor;
        }

        const std::set<std::string>& GetParentClasses() const
        {
            return ParentClasses;
        }

        const std::set<std::string>& GetDerivedClasses() const
        {
            return DerivedClasses;
        }

    private:
        std::string TypeName;
        std::unique_ptr<Constructor> DefaultCtor;
        std::set<std::string> ParentClasses;
        std::set<std::string> DerivedClasses;
        std::map<std::string, std::unique_ptr<MemberVariable>> MemberVariables;
        std::map<std::string, std::unique_ptr<MemberFunction>> MemberFunctions;
    };

}