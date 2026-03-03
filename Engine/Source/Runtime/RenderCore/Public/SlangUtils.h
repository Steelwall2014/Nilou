#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include "HAL/Platform.h"
#include "RHIDefinitions.h"
#include "ShaderCompileEnvironment.h"

namespace nilou {

class FSlangUtils
{
public:
    static RENDERCORE_API void PrintProgramLayout(slang::ProgramLayout* ProgramLayout, SlangCompileTarget TargetFormat);
    static RENDERCORE_API void PrintVariable(slang::VariableReflection* Variable);
    static RENDERCORE_API void PrintType(slang::TypeReflection* Type);
    static RENDERCORE_API void PrintVariableLayout(slang::VariableLayoutReflection* VariableLayout);
    static RENDERCORE_API void PrintTypeLayout(slang::TypeLayoutReflection* TypeLayout);
    static RENDERCORE_API EDescriptorType TranslateBindingTypeToDescriptorType(slang::BindingType BindingType);
};
    

}
