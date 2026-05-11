#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <slang-com-ptr.h>
#include <slang.h>

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

bool MapEquals(const std::map<std::string, long long>& map1, const std::map<std::string, long long>& map2)
{
    if (map1.size() != map2.size())
    {
        return false;
    }
    return std::equal(map1.begin(), map1.end(), map2.begin());
}

void CompareAndEmit(const fs::path& outputFilePath, const std::string& content)
{
    {
        std::ifstream existingFile(outputFilePath);
        if (existingFile.is_open())
        {
            std::string existingContent((std::istreambuf_iterator<char>(existingFile)), std::istreambuf_iterator<char>());
            if (existingContent == content)
            {
                return;
            }
        }
    }
    std::ofstream outFile(outputFilePath);
    if (outFile.is_open())
    {
        outFile << content;
    }
    else
    {
        std::cout << "Failed to open output file: " << outputFilePath << std::endl;
    }
    std::cout << "Generated: " << outputFilePath << std::endl;
}

std::string MakeStaleGeneratedHeaderPlaceholder(const std::string& stem)
{
    return std::format(
        "#pragma once\n"
        "// This file is intentionally empty. Shader module \"{}\" no longer has\n"
        "// reflectable ParameterBlock<T> bindings, but the placeholder is kept so\n"
        "// existing build graphs and includes do not reference a missing file.\n",
        stem);
}

std::string MakeStaleGeneratedCppPlaceholder(const std::string& stem)
{
    return std::format(
        "// This file is intentionally empty. Shader module \"{}\" no longer has\n"
        "// reflectable ParameterBlock<T> bindings, but the placeholder is kept so\n"
        "// xmake does not compile a source file that was removed during codegen.\n",
        stem);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: NilouShaderTool -InputDirectory=<directory> -OutputHeaderDirectory=<directory> -OutputCppDirectory=<directory> [-SearchDirectories=<directory>;<directory>...]" << std::endl;
        return -1;
    }

    fs::path InputDirectory;
    fs::path OutputHeaderDirectory;
    fs::path OutputCppDirectory;
    std::vector<std::string> SearchDirectories;
    bool bForceRegenerate = false;
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        
        if (arg.find("-InputDirectory=") == 0)
        {
            InputDirectory = arg.substr(16); // 16 = length of "-InputDirectory="
        }
        else if (arg.find("-OutputHeaderDirectory=") == 0)
        {
            OutputHeaderDirectory = arg.substr(23); // 23 = length of "-OutputHeaderDirectory="
        }
        else if (arg.find("-OutputCppDirectory=") == 0)
        {
            OutputCppDirectory = arg.substr(20); // 20 = length of "-OutputCppDirectory="
        }
        else if (arg.find("-SearchDirectories=") == 0)
        {
            std::string SearchDirectoriesString = arg.substr(19); // 19 = length of "-SearchDirectories="
            SearchDirectories = Split(SearchDirectoriesString, ';');
        }
        else if (arg.find("-ForceRegenerate") == 0)
        {
            bForceRegenerate = true;
        }
    }

    if (InputDirectory.empty() || OutputHeaderDirectory.empty() || OutputCppDirectory.empty())
    {
        std::cout << "Usage: NilouShaderTool -InputDirectory=<directory> -OutputHeaderDirectory=<directory> -OutputCppDirectory=<directory>" << std::endl;
        return -1;
    }

    if (!fs::exists(OutputHeaderDirectory))
    {
        fs::create_directories(OutputHeaderDirectory);
    }
    if (!fs::exists(OutputCppDirectory))
    {
        fs::create_directories(OutputCppDirectory);
    }

    if (!fs::exists(InputDirectory))
    {
        std::cout << "Input directory does not exist: " << InputDirectory << std::endl;
        return -1;
    }

    fs::path CachedShaderModifiedTimePath = OutputHeaderDirectory / "CachedShaderModifiedTime.txt";
    
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

    
    std::map<std::string, long long> CachedShaderModifiedTime;
    std::map<std::string, long long> CurrentShaderModifiedTime;
    if (!bForceRegenerate)
    {
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
    }

    {
        fs::path ExePath = fs::canonical(argv[0]);
        std::string ExePathString = ExePath.generic_string();
        long long ExeModifiedTime = fs::last_write_time(ExePath).time_since_epoch().count();
        CurrentShaderModifiedTime[ExePathString] = ExeModifiedTime;
    }

    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(InputDirectory))
    {
        if (!dir_entry.is_directory())
        {
            fs::path SlangFilePath = dir_entry.path();
            std::string SlangFilePathString = SlangFilePath.generic_string();
            long long last_modified_time = fs::last_write_time(SlangFilePath).time_since_epoch().count();
            CurrentShaderModifiedTime[SlangFilePathString] = last_modified_time;
        }
    }

    if (MapEquals(CurrentShaderModifiedTime, CachedShaderModifiedTime))
    {
        std::cout << "[NilouShaderTool] All shader files are up-to-date." << std::endl;
        return 0;
    }

    SlangShaderReflectionSession reflectionSession(Session.get());
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(InputDirectory))
    {
        if (!dir_entry.is_directory() && IsSlangModule(dir_entry.path()))
        {
            fs::path SlangFilePath = dir_entry.path();
            std::string SlangFilePathString = SlangFilePath.generic_string();
            std::cout << "Load Module: " << SlangFilePathString << std::endl;
            reflectionSession.LoadModule(SlangFilePath);
        }
    }

    reflectionSession.EmitCppStructs();

    std::unordered_map<std::string, std::vector<slang::TypeReflection*>> FileToTypesMap;
    for (auto& TypeDecl : reflectionSession.TypeDeclarations)
    {
        std::string filename = fs::path(TypeDecl.SourceLocation.filePath).stem().string();
        if (TypeDecl.bUsedInParameterBlock)
        {
            FileToTypesMap[filename].push_back(TypeDecl.Type);
        }
    }

    for (auto& [filename, TypesInThisFile] : FileToTypesMap)
    {
        const fs::path outputHeaderFilePath = OutputHeaderDirectory / (filename + ".generated.h");
        const fs::path outputCppFilePath = OutputCppDirectory / (filename + ".gen.cpp");

        {
            std::stringstream Declarations;
            Declarations << "#pragma once\n"
                         << "#include \"RenderGraphResources.h\"\n"
                         << "#include \"ShaderParameter.h\"\n"
                         << "namespace nilou {\n"
                         << "namespace shader {\n"
                         << "\n";
            for (slang::TypeReflection* Type : TypesInThisFile)
            {
                Declarations << reflectionSession.GetCppStructDeclaration(Type);
            }
            Declarations << "\n"
                         << "} // namespace shader\n"
                         << "} // namespace nilou\n";
            CompareAndEmit(outputHeaderFilePath, Declarations.str());
        }

        {
            std::stringstream Definitions;
            Definitions << std::format("#include \"{}\"\n", filename + ".generated.h")
                        << "#include \"DynamicRHI.h\"\n"
                        << "namespace nilou {\n"
                        << "\n";
            for (slang::TypeReflection* Type : TypesInThisFile)
            {
                Definitions << reflectionSession.GetCppStructDefinition(Type);
            }
            Definitions << "\n"
                        << "} // namespace nilou\n";
            CompareAndEmit(outputCppFilePath, Definitions.str());
        }
    }

    for (const fs::directory_entry& dir_entry : fs::directory_iterator(OutputHeaderDirectory))
    {
        if (!dir_entry.is_directory() && dir_entry.path().string().ends_with(".generated.h"))
        {
            // "TestGeneration.generated.h" -> stem() -> "TestGeneration.generated" -> stem() -> "TestGeneration"
            std::string stem = dir_entry.path().stem().stem().string();
            if (FileToTypesMap.find(stem) == FileToTypesMap.end())
            {
                CompareAndEmit(dir_entry.path(), MakeStaleGeneratedHeaderPlaceholder(stem));
            }
        }
    }
    for (const fs::directory_entry& dir_entry : fs::directory_iterator(OutputCppDirectory))
    {
        if (!dir_entry.is_directory() && dir_entry.path().string().ends_with(".gen.cpp"))
        {
            // "TestGeneration.gen.cpp" -> stem() -> "TestGeneration.gen" -> stem() -> "TestGeneration"
            std::string stem = dir_entry.path().stem().stem().string();
            if (FileToTypesMap.find(stem) == FileToTypesMap.end())
            {
                CompareAndEmit(dir_entry.path(), MakeStaleGeneratedCppPlaceholder(stem));
            }
        }
    }

    std::ofstream outShaderModifiedTime(CachedShaderModifiedTimePath.string());
    for (auto& [filename, last_modified_time] : CurrentShaderModifiedTime)
    {
        outShaderModifiedTime << filename << " " << last_modified_time << "\n";
    }
    outShaderModifiedTime.close();

    return 0;
}
