#include "Directory.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace HeaderTool;

string relative_path(const char *from, const char *to)
{
    string result;
    int i,j,k;
    i = j = 0;
    while(from[i] != '\0' && to[i] != '\0' && from[i] == to[i]){
        i++;
    }
    k = i;
    while(from[k] != '\0'){
        if(from[k++] == '/'){
            result.push_back('.');
            result.push_back('.');
            result.push_back('/');
        }else{
            continue;
        }
    }
    while(to[i] != '\0'){
        result.push_back(to[i++]);
    }
    return result;
}

bool StartsWith(const std::string &str, const std::string &temp)
{
    if (str.size() < temp.size())
        return false;
    int length = std::min(str.size(), temp.size());
    for (int i = 0; i < length; i++)
    {
        if (str[i] != temp[i])
            return false;
    }
    return true;
}

bool EndsWith(const std::string &str, const std::string &temp)
{
    if (str.size() < temp.size())
        return false;
    int length = std::min(str.size(), temp.size());
    for (int i = 0; i < length; i++)
    {
        if (str[str.size()-1 - i] != temp[temp.size()-1 - i])
            return false;
    }
    return true;
}

void Trim(std::string &s)
{
    if( !s.empty() )
    {
        if (s[0] == ' ')
        {
            s.erase(0, s.find_first_not_of(" "));
            s.erase(s.find_last_not_of(" ") + 1);
            s.erase(0, s.find_first_not_of("\t"));
            s.erase(s.find_last_not_of("\t") + 1);
        }
        else if (s[0] == '\t')
        {
            s.erase(0, s.find_first_not_of("\t"));
            s.erase(s.find_last_not_of("\t") + 1);
            s.erase(0, s.find_first_not_of(" "));
            s.erase(s.find_last_not_of(" ") + 1);
        }
    }
}
size_t find_first_not_delim(const std::string &s, char delim, size_t pos)
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
    if (argc != 3)
    {
        cout << "Usage: HeaderTool <src directory> <generated code directory>" << endl;
        return -1;
    }
    std::string DirectoryName = argv[1];
    //std::string DirectoryName = "./src"; //argv[1];
    if (DirectoryName[DirectoryName.size()-1] == '\\' || DirectoryName[DirectoryName.size()-1] == '/')
        DirectoryName.substr(0, DirectoryName.size()-1);
    std::string GeneratedCodePath = argv[2];
    //std::string GeneratedCodePath = "./src/Runtime/Generated"; //argv[2];
    if (GeneratedCodePath[GeneratedCodePath.size()-1] == '\\' || GeneratedCodePath[GeneratedCodePath.size()-1] == '/')
        GeneratedCodePath.substr(0, GeneratedCodePath.size()-1);

    string rel_path = relative_path(GeneratedCodePath.c_str(), DirectoryName.c_str());

    std::string ImplementationBody;

    std::string MarkedClassesEnumBody;

    std::string IncludedPaths;

    std::vector<string> ClassNames;

    Directory dir(DirectoryName);
    dir.ForEachFile(true, [&ClassNames, &IncludedPaths, &ImplementationBody, GeneratedCodePath, &MarkedClassesEnumBody, rel_path] (File File) {
        if (EndsWith(File.GetFilePath(), ".h"))
        {
            vector<string> Lines = File.ReadLines();
            for (int i = 0; i < Lines.size(); i++)
            {
                Trim(Lines[i]);
            }
            for (int i = 0; i < Lines.size(); i++)
            {
                string &Line = Lines[i];
                if (StartsWith(Line, "UCLASS()"))
                {
                    Line = Lines[i+1];
                    if (!StartsWith(Line, "class"))
                        return;
                    vector<string> tokens = Split(Line, ':');
                    string ClassName;
                    if (tokens.size() == 1)
                    {
                        Trim(tokens[0]);
                        ClassName = Split(tokens[0], ' ')[1];
                        ImplementationBody += "\tAddNode(EUClasses::MC_" + ClassName + ");\n";
                    }
                    else
                    {
                        string &BeforeColon = tokens[0];
                        string &AfterColon = tokens[1];
                        ClassName = Split(tokens[0], ' ')[1];
                        vector<string> ParentClassNames = Split(AfterColon, ',');
                        for (string &ParentClassName : ParentClassNames)
                        {
                            Trim(ParentClassName);
                            if (StartsWith(ParentClassName, "public") || 
                                StartsWith(ParentClassName, "protected") ||
                                StartsWith(ParentClassName, "private"))
                            {
                                ParentClassName = Split(ParentClassName, ' ')[1];
                            }
                            ImplementationBody += "\tAddEdge(EUClasses::MC_" + ParentClassName + ", EUClasses::MC_" + ClassName + ");\n";
                        }
                    }
                    ClassNames.push_back(ClassName);

                    string file_name = File.GetFilePath();
                    file_name.erase(0, file_name.find_last_of("/"));
                    file_name.erase(file_name.find_last_of("."));

                    /********** SomeClass.generated.cpp **********/
                    ofstream out_stream(GeneratedCodePath + "/" + ClassName + ".generated.cpp", ios::out);
                    string filepath = File.GetFilePath();
                    if (filepath[0] == '.' && filepath[1] == '/')
                        filepath.erase(0, 2);
                    std::filesystem::path ProjectDir = PROJECT_DIR;
                    IncludedPaths += "#include \"" + /*rel_path + */filepath + "\"\n";
                    string content = 
                        "#include \"" + /*rel_path + */filepath + "\"\n"
                        "namespace nilou {\n"
                        "std::string " + ClassName + "::GetClassName() { return \"" + ClassName + "\"; }\n"
                        "EUClasses " + ClassName + "::GetClassEnum() { return EUClasses::MC_" + ClassName + "; }\n"
                        "const UClass *" + ClassName + "::GetClass() { return " + ClassName + "::StaticClass(); }\n"
                        "const UClass *" + ClassName + "::StaticClass()\n"
                        "{\n"
                        "\tstatic UClass *StaticClass = new UClass(\"" + ClassName + "\", EUClasses::MC_" + ClassName + ");\n"
                        "\treturn StaticClass;\n"
                        "}\n"
                        "std::unique_ptr<UObject> "+ClassName+"::CreateDefaultObject()\n"
                        "{\n    return std::make_unique<"+ClassName+">();\n}\n"
                        "}\n";

                    out_stream << content;
                    /********** SomeClass.generated.cpp **********/

                    MarkedClassesEnumBody += "\tMC_" + ClassName + ",\n";
                }
            }
        }
    });

    /********** InheritanceGraph.generated.cpp **********/
    ofstream out_stream(GeneratedCodePath + "/InheritanceGraph.generated.cpp", ios::out);
    out_stream << 
        "#include \"Common/InheritanceGraph.h\"\n"
        "namespace nilou {\n"
        "FInheritanceGraph *FInheritanceGraph::GetInheritanceGraph()\n"
        "{\n"
        "    static FInheritanceGraph *InheritanceGraph = new FInheritanceGraph;\n"
        "    return InheritanceGraph;\n"
        "} \n"
        "FInheritanceGraph::FInheritanceGraph() {\n" << 
        ImplementationBody << 
        "}\n}";
    /********** InheritanceGraph.generated.cpp **********/

    /********** MarkedClasses.generated.h **********/
    out_stream = ofstream(GeneratedCodePath + "/MarkedClasses.generated.h", ios::out);
    out_stream 
        << "#pragma once\n"
            "namespace nilou {\n"
            "enum class EUClasses {\n"
           "\tMC_None = -1,\n"
        << MarkedClassesEnumBody 
        << "\tMC_UClassNum,\n"
           "};\n}";
    /********** MarkedClasses.generated.h **********/

    /********** ObjectFactory.generated.cpp **********/
    out_stream = ofstream(GeneratedCodePath + "/ObjectFactory.generated.cpp", ios::out);
    out_stream 
        << IncludedPaths << 
        "using namespace nilou;\n"
        "FObjectFactory::FObjectFactory()\n"
        "{\n";
        for (auto &ClassName : ClassNames)
        {
            out_stream << "    FunctionMap[\""+ClassName+"\"] = &"+ClassName+"::CreateDefaultObject;\n";
        }
        out_stream << "}";
    /********** ObjectFactory.generated.cpp **********/

    return 0;
}