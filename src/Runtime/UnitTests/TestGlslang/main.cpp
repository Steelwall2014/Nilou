#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace glslang;

class MyTraverser : public TIntermTraverser
{
public:

    virtual void visitSymbol(TIntermSymbol*inter)               
    { 
        TString str = inter->getAccessName();
        if (str == "LODParams")
            std::cout << 1;
    }
    virtual void visitConstantUnion(TIntermConstantUnion*inter) 
    { 
        const TType &str = inter->getType();
    }
    virtual bool visitBinary(TVisit, TIntermBinary*inter)       
    { 
        const TType &str = inter->getType();
        return true;
    }
    virtual bool visitUnary(TVisit, TIntermUnary*inter)         
    { 
        const TType &str = inter->getType();
        return true;
    }
    virtual bool visitSelection(TVisit, TIntermSelection*inter) 
    { 
        const TType &str = inter->getType();
        return true;
    }
    virtual bool visitAggregate(TVisit, TIntermAggregate*inter) 
    { 
        const TType &str = inter->getType();
        return true;
    }
    virtual bool visitLoop(TVisit, TIntermLoop*inter)           
    { 
        return true;
    }
    virtual bool visitBranch(TVisit, TIntermBranch*inter)       
    { 
        return true;
    }
    virtual bool visitSwitch(TVisit, TIntermSwitch*inter)       
    { 
        return true;
    }

};

int main()
{
    glslang::InitializeProcess();
    TShader *Shader = new TShader(EShLanguage::EShLangCompute);
    std::ifstream in{"D:\\Nilou\\Assets\\Shaders\\VirtualHeightfieldMesh\\VHM_create_lod_texture.comp"};
    char buffer[1024];
    std::stringstream res;
    while (in.getline(buffer, sizeof(buffer)))
    {
        res << buffer << "\n";
    }
    std::string code = res.str();
    const char *s = code.c_str();
    Shader->setStrings(&s, 1);
    Shader->setEnvInput(EShSourceGlsl , EShLanguage::EShLangCompute,  EShClientNone, 0);
    Shader->setEnvClient(EShClientNone, EShTargetClientVersion(0));
    Shader->setEnvTarget(EShTargetNone, EShTargetLanguageVersion(0));
    EShMessages messages = EShMsgDefault;
    std::string preprocess;
    *GetResources() = *GetDefaultResources();
    GetResources()->maxAtomicCounterBindings = 2;

    TShader::ForbidIncluder includer;
    bool result = Shader->preprocess(GetResources(), 100, EProfile::ECoreProfile, false, false, messages, &preprocess, includer);
    result = Shader->parse(GetResources(), 100, false, messages);
    TProgram program;
    program.addShader(Shader);
    result = program.link(messages);
    std::cout << program.getInfoDebugLog();
    std::cout << program.getInfoLog();

    program.buildReflection();

    int Buffer_block_num = program.getNumBufferBlocks();
    for (int i = 0; i < Buffer_block_num; i++)
    {
        auto &obj = program.getBufferBlock(i);
        int j = obj.getBinding();
    }

    // auto sampler = program.getUniform(0);

    int uniform_num = program.getNumUniformVariables();
    for (int i = 0; i < uniform_num; i++)
    {
        auto name = program.getUniformName(i);
        auto binding = program.getUniformBinding(i);
        auto obj = program.getUniform(i);
        bool b = obj.getType()->isImage();
        std::cout << name;
    }

    int uniform_block_num = program.getNumUniformBlocks();
    for (int i = 0; i < uniform_block_num; i++)
    {
        auto name = program.getUniformBlockName(i);
        auto binding = program.getUniformBlockBinding(i);
        auto obj = program.getUniformBlock(i);
        std::cout << name;
    }

}