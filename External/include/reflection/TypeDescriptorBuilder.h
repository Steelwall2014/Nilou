#pragma once
#include <memory>
#include "TypeDescriptor.h"
#include "Registry.h"

namespace reflection {

    class RawTypeDescriptorBuilder 
    {
    public:
        explicit RawTypeDescriptorBuilder(const std::string& name)
            : Descriptor(std::make_unique<TypeDescriptor>()) 
        {
            Descriptor->TypeName = name;
        }
        RawTypeDescriptorBuilder(const RawTypeDescriptorBuilder&) = delete;
        ~RawTypeDescriptorBuilder() 
        {
            Registry::Instance().Register(std::move(Descriptor));
        }

        template<class TClass>
        void AddDefaultConstructor() 
        {
            Descriptor->DefaultCtor = std::make_unique<Constructor>((TClass*)nullptr);
        }

        template<class TClass, typename TVar>
        void AddMemberVariable(const std::string& name, TVar TClass::* var) 
        {
            auto variable = std::make_unique<MemberVariable>(var);
            variable->MemberName = name;
            Descriptor->MemberVariables[name] = std::move(variable);
        }

        template<class TClass, typename TFunc>
        void AddMemberFunction(const std::string& name, TFunc TClass::* func) 
        {
            auto function = std::make_unique<MemberFunction>(func);
            function->FunctionName = name;
            Descriptor->MemberFunctions[name] = std::move(function);
        }

        void AddParentClass(const std::string& ParentClass) 
        {
            Descriptor->ParentClasses.insert(ParentClass);
        }

        void AddDerivedClass(const std::string& DerivedClass) 
        {
            Descriptor->DerivedClasses.insert(DerivedClass);
        }

    private:
        std::unique_ptr<TypeDescriptor> Descriptor = nullptr;
    };

    template<class TClass>
    class TypeDescriptorBuilder 
    {
    public:
        explicit TypeDescriptorBuilder(const std::string& name) : RawBuilder(name) {}

        TypeDescriptorBuilder& AddDefaultConstructor() 
        {
            RawBuilder.AddDefaultConstructor<TClass>();
            return *this;
        }

        template<typename TVar>
        TypeDescriptorBuilder& AddMemberVariable(const std::string& name, TVar TClass::* var) 
        {
            RawBuilder.AddMemberVariable(name, var);
            return *this;
        }

        template<typename TFunc>
        TypeDescriptorBuilder& AddMemberFunction(const std::string& name, TFunc TClass::* func) 
        {
            RawBuilder.AddMemberFunction(name, func);
            return *this;
        }

        TypeDescriptorBuilder& AddParentClass(const std::string& ParentClass) 
        {
            RawBuilder.AddParentClass(ParentClass);
            return *this;
        }

        TypeDescriptorBuilder& AddDerivedClass(const std::string& DerivedClass) 
        {
            RawBuilder.AddDerivedClass(DerivedClass);
            return *this;
        }

    private:
        RawTypeDescriptorBuilder RawBuilder;
    };
    
    template<class TClass>
    TypeDescriptorBuilder<TClass> AddClass(const std::string& name) 
    {
        return TypeDescriptorBuilder<TClass>(name);
    }
}