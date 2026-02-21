#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <map>
#include <slang.h>
#include <slang-com-ptr.h>
#include "ShaderReflection.h"
#include "utils.h"

namespace fs = std::filesystem;

static size_t find_first_not_delim(const std::string &s, char delim, size_t pos)
{
    for (size_t i = pos; i < s.size(); i++)
        if (s[i] != delim)
            return i;
    return std::string::npos;
}

std::vector<std::string> Split(const std::string &s, char delim)
{
    std::vector<std::string> tokens;
    size_t lastPos = find_first_not_delim(s, delim, 0);
    size_t pos = s.find(delim, lastPos);
    while (lastPos != std::string::npos)
    {
        tokens.push_back(s.substr(lastPos, pos - lastPos));
        lastPos = find_first_not_delim(s, delim, pos);
        pos = s.find(delim, lastPos);
    }
    return tokens;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: NilouShaderTool -InputDirectory=<directory> -OutputDirectory=<directory> [-SearchDirectories=<directory>,<directory>...]" << std::endl;
        return -1;
    }

    fs::path InputDirectory;
    fs::path OutputDirectory;
    std::vector<std::string> SearchDirectories;
    bool bForceRegenerate = false;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        
        if (arg.find("-InputDirectory=") == 0)
        {
            InputDirectory = arg.substr(16); // 16 = length of "-InputDirectory="
        }
        else if (arg.find("-OutputDirectory=") == 0)
        {
            OutputDirectory = arg.substr(17); // 17 = length of "-OutputDirectory="
        }
        else if (arg.find("-SearchDirectories=") == 0)
        {
            std::string SearchDirectoriesString = arg.substr(19); // 19 = length of "-SearchDirectories="
            SearchDirectories = Split(SearchDirectoriesString, ',');
        }
        else if (arg.find("-ForceRegenerate") == 0)
        {
            bForceRegenerate = true;
        }
    }

    if (InputDirectory.empty() || OutputDirectory.empty())
    {
        std::cout << "Usage: NilouShaderTool -InputDirectory=<directory> -OutputDirectory=<directory>" << std::endl;
        return -1;
    }

    if (!fs::exists(OutputDirectory))
    {
        fs::create_directories(OutputDirectory);
    }

    if (!fs::exists(InputDirectory))
    {
        std::cout << "Input directory does not exist: " << InputDirectory << std::endl;
        return -1;
    }

    std::map<std::string, long long> CachedShaderModifiedTime;
    fs::path CachedShaderModifiedTimePath = fs::path(OutputDirectory) / fs::path("CachedShaderModifiedTime.txt");
    if (fs::exists(CachedShaderModifiedTimePath))
    {
        std::ifstream in{CachedShaderModifiedTimePath.string()};
        while (!in.eof())
        {
            std::string filename;
            long long last_modified_time;
            in >> filename >> last_modified_time;
            if (filename != "")
            {
                CachedShaderModifiedTime[filename] = last_modified_time;
            }
        }
    }
    
    Slang::ComPtr<slang::IGlobalSession> SlangGlobalSession = nullptr;
    if (SlangGlobalSession == nullptr)
    {
        slang::createGlobalSession(SlangGlobalSession.writeRef());
    }

    slang::SessionDesc sessionDesc = {};
    std::vector<slang::TargetDesc> targets = {
        {
            .format = SLANG_SPIRV,
            .profile = SlangGlobalSession->findProfile("spirv_1_5")
        },
    };
    std::string InputDirectoryString = InputDirectory.generic_string();
    std::vector<const char*> searchPaths = { 
        InputDirectoryString.c_str(), 
    };
    for (const std::string& SearchDirectory : SearchDirectories)
    {
        searchPaths.push_back(SearchDirectory.c_str());
    }
    sessionDesc.targetCount = (SlangInt)targets.size();
    sessionDesc.targets = targets.data();
    sessionDesc.searchPathCount = searchPaths.size();
    sessionDesc.searchPaths = searchPaths.data();

    Slang::ComPtr<slang::ISession> Session;
    SlangGlobalSession->createSession(sessionDesc, Session.writeRef());

    
    bool bHasChangedFiles = false;
    SlangShaderReflectionSession reflectionSession(Session.get());
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(InputDirectory))
    {
        if (!dir_entry.is_directory() && IsSlangModule(dir_entry.path()))
        {
            fs::path SlangFilePath = dir_entry.path();
            long long cached_last_modified_time = CachedShaderModifiedTime[SlangFilePath.generic_string()];
            long long last_modified_time = fs::last_write_time(SlangFilePath).time_since_epoch().count();
            if (cached_last_modified_time == 0 || last_modified_time != cached_last_modified_time || bForceRegenerate)
            {
                bHasChangedFiles = true;
                CachedShaderModifiedTime[SlangFilePath.generic_string()] = last_modified_time;
                std::cout << "Processing: " << SlangFilePath.generic_string() << std::endl;
                reflectionSession.LoadModule(SlangFilePath);
            }
        }
    }

    reflectionSession.EmitCppStructs();

    std::unordered_map<std::string, std::vector<std::string>> FileToTypesMap;
    for (auto& TypeDecl : reflectionSession.TypeDeclarations)
    {
        FileToTypesMap[TypeDecl.SourceLocation.filePath].push_back(TypeDecl.TypeName);
    }

    for (auto& [File, TypesInThisFile] : FileToTypesMap)
    {
        std::string filename = fs::path(File).stem().string();
        std::string outputHeaderFilePath = (OutputDirectory / (filename + ".generated.h")).generic_string();
        std::string outputCppFilePath = (OutputDirectory / (filename + ".gen.cpp")).generic_string();

        {
            std::string Result;
            for (const std::string& TypeName : TypesInThisFile)
            {
                auto& TypeDecl = reflectionSession.GetTypeDeclaration(TypeName);
                Result += std::format("// Begin {}\n", TypeName);
                Result += std::format("template <EShaderDataLayout DataLayout> struct {} {{}};\n", TypeName);
                for (auto& [DataLayout, CppStruct] : TypeDecl.CppStructs)
                {
                    Result += CppStruct;
                }
                Result += std::format("// End {}\n\n\n", TypeName);
            }
            std::ofstream out(outputHeaderFilePath);
            if (out.is_open())
            {
                out << "#pragma once\n"
                    << "#include \"RenderGraphResources.h\"\n"
                    << "#include \"ShaderParameter.h\"\n"
                    << "namespace nilou {\n"
                    << "\n"
                    << Result
                    << "\n"
                    << "}\n";
                out.close();
                std::cout << "Generated: " << outputHeaderFilePath << std::endl;
            }
            else
            {
                std::cout << "Failed to open output file: " << outputHeaderFilePath << std::endl;
            }
        }

        {
            std::string Result;
            for (const std::string& TypeName : TypesInThisFile)
            {
                Result += std::format("// Begin {}\n", TypeName);
                auto& TypeDecl = reflectionSession.GetTypeDeclaration(TypeName);
                Result += TypeDecl.CppMetadata;
                Result += std::format("// End {}\n\n\n", TypeName);
            }
            std::ofstream out(outputCppFilePath);
            if (out.is_open())
            {
                out << std::format("#include \"{}\"\n", filename + ".generated.h")
                    << "#include \"DynamicRHI.h\"\n"
                    << "namespace nilou {\n"
                    << "\n"
                    << Result
                    << "\n"
                    << "}\n";
                out.close();
                std::cout << "Generated: " << outputCppFilePath << std::endl;
            }
            else
            {
                std::cout << "Failed to open output file: " << outputCppFilePath << std::endl;
            }
        }
    }

    if (bHasChangedFiles)
    {
        std::ofstream out{CachedShaderModifiedTimePath.string()};
        for (auto& [filename, last_modified_time] : CachedShaderModifiedTime)
        {
            out << filename << " " << last_modified_time << "\n";
        }
    }
    else
    {
        std::cout << "[NilouShaderTool] All shader files are up-to-date.\n";
    }

    return 0;
}
