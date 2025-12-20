#include "slang.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include <array>
#include <iostream>
#include <fstream>
#include <vector>


void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
    if (diagnosticsBlob != nullptr)
    {
        std::cout << (const char*)diagnosticsBlob->getBufferPointer() << std::endl;
        __debugbreak();
    }
}

int main()
{
    // 1. Create Global Session
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    createGlobalSession(globalSession.writeRef());

    // 2. Create Session
    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    // std::array<slang::CompilerOptionEntry, 1> options = 
    //     {
    //         {
    //             slang::CompilerOptionName::EmitSpirvDirectly,
    //             {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
    //         }
    //     };
    // sessionDesc.compilerOptionEntries = options.data();
    // sessionDesc.compilerOptionEntryCount = options.size();

    Slang::ComPtr<slang::ISession> session;
    globalSession->createSession(sessionDesc, session.writeRef());

    // 3. Load module
    Slang::ComPtr<slang::IModule> MainModule;
    {
        std::string path = "../../../../Engine/Source/Programs/TestSlang/Main.slang";
        std::ifstream file(path);
        std::string shaderSource((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        MainModule = session->loadModuleFromSourceString(
            "Main",                  // Module name
            path.c_str(),            // Module path
            shaderSource.c_str(),              // Shader source code
            diagnosticsBlob.writeRef()); // Optional diagnostic container
        diagnoseIfNeeded(diagnosticsBlob);
        if (!MainModule)
        {
            return -1;
        }
    }

    Slang::ComPtr<slang::IModule> ImplementationModule;
    {
        std::string path = "../../../../Engine/Source/Programs/TestSlang/Implementation.slang";
        std::ifstream file(path);
        std::string shaderSource((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        ImplementationModule = session->loadModuleFromSourceString(
            "Implementation",                  // Module name
            path.c_str(),            // Module path
            shaderSource.c_str(),              // Shader source code
            diagnosticsBlob.writeRef()); // Optional diagnostic container
        diagnoseIfNeeded(diagnosticsBlob);
        if (!ImplementationModule)
        {
            return -1;
        }
    }

    // 4. Query Entry Points
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        MainModule->findEntryPointByName("computeMain", entryPoint.writeRef());
        if (!entryPoint)
        {
            std::cout << "Error getting entry point" << std::endl;
            return -1;
        }
    }
    
    std::vector<slang::SpecializationArg> specializationArgs =
    {
        {
            slang::SpecializationArg::Kind::Type,
            ImplementationModule->getLayout()->findTypeByName("Implementation")
        },
        {
            slang::SpecializationArg::Kind::Type,
            ImplementationModule->getLayout()->findTypeByName("ImplementationInput")
        }
    };

    Slang::ComPtr<slang::IComponentType> specializedEntryPoint;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = entryPoint->specialize(
            specializationArgs.data(),
            specializationArgs.size(),
            specializedEntryPoint.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
    }

    // 5. Compose Modules + Entry Points
    std::vector<slang::IComponentType*> componentTypes =
        {
            MainModule,
            ImplementationModule,
            specializedEntryPoint
        };

    Slang::ComPtr<slang::IComponentType> composedProgram;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = session->createCompositeComponentType(
            componentTypes.data(),
            componentTypes.size(),
            composedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);
    }

    // 6. Link
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = composedProgram->link(
            linkedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);
    }

    // 7. Get Target Kernel Code
    Slang::ComPtr<slang::IBlob> spirvCode;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = linkedProgram->getEntryPointCode(
            0,
            0,
            spirvCode.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);
    }

    // Test reflection
    slang::ProgramLayout* ProgramLayout = linkedProgram->getLayout();
    int parameterCount = ProgramLayout->getParameterCount();
    for (int i = 0; i < parameterCount; i++)
    {
        slang::VariableLayoutReflection* VarLayout = ProgramLayout->getParameterByIndex(i);
        if (VarLayout == nullptr)
            continue;

        std::string name = VarLayout->getName();
        std::cout << "Name: " << name << std::endl;
    }
    slang::TypeLayoutReflection* TypeLayout = ProgramLayout->getGlobalParamsTypeLayout();
    if (TypeLayout)
    {
        int fieldCount = TypeLayout->getFieldCount();
        for (int i = 0; i < fieldCount; i++)
        {
            slang::VariableLayoutReflection* VarLayout = TypeLayout->getFieldByIndex(i);
            if (VarLayout == nullptr)
                continue;

            uint32_t descriptorSetIndex = VarLayout->getOffset(slang::ParameterCategory::RegisterSpace);
            std::string name = VarLayout->getName();
            std::cout << "Name: " << name << ", Descriptor Set Index: " << descriptorSetIndex << std::endl;
        }
    }


    std::cout << "Compiled " << spirvCode->getBufferSize() << " bytes of SPIR-V" << std::endl;
}