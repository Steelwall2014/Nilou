#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include "clang-c/Index.h"
#include <regex>
#include <execution>
#include <format>
 
#include "utils.h"

using namespace std;

class Directory
{
public:
    Directory(const std::string &InDirectoryName) : DirectoryName(InDirectoryName) { }
    const std::string &GetDirectoryName() const;

    template <typename Func>
    void ForEachFile(bool bFindInChildren, Func&& InFunc)
    {
        if (!std::filesystem::exists(DirectoryName))
        {
            std::cout << "Directory: " + DirectoryName + " doesn't exist" << std::endl;
            return;
        }
            
        for (const std::filesystem::directory_entry & dir_entry : 
            std::filesystem::recursive_directory_iterator(DirectoryName))
        {
            if (!dir_entry.is_directory())
            {
                std::string filepath = dir_entry.path().generic_string();
                InFunc(filepath);
            }
        }
    }

private:

    std::string DirectoryName;
};

ostream& operator<<(ostream& stream, const CXString& str)
{
    const char* s = clang_getCString(str);
    stream << s;
    clang_disposeString(str);
    return stream;
}

std::vector<const char*> arguments = {
    "-x",
    "c++",
    "-std=c++20",
    "-D __clang__",
    "-D __META_PARSER__"
};

struct TypeMetaData
{
    string FileName;
    string Name;
    string BaseClass;
    set<string> DerivedClasses;
    map<string, std::pair<bool, string>> Fields;
    set<string> Methods;
    vector<vector<string>> Constructors;
    string GeneratedFileCode;
    string MetaType; // class or struct
};
map<string, TypeMetaData> NTypes;

string fully_qualified(CXCursor c)
{
    if (clang_getCursorKind(c) == CXCursorKind::CXCursor_TranslationUnit || 
        clang_getCursorKind(c) == CXCursorKind::CXCursor_FirstInvalid)
        return "";
    else
    {
        string res = fully_qualified(clang_getCursorSemanticParent(c));
        if (res != "")
            return res + "::" + GetCursorSpelling(c);
    }
    return GetCursorSpelling(c);
}

bool IsReflectedStruct(const string& TypeName)
{
    return NTypes.contains(TypeName) && NTypes[TypeName].MetaType == "struct";
}

bool IsReflectedClass(const string& TypeName)
{
    return NTypes.contains(TypeName) && NTypes[TypeName].MetaType == "class";
}

bool IsReflectedType(const std::string& TypeName)
{
    return NTypes.contains(TypeName);
}

bool IsNClassPtr(CXType Type)
{
    CXType pointee = clang_getPointeeType(Type);
    return IsReflectedClass(GetClangString(clang_getTypeSpelling(pointee)));
}

bool IsNClassSmartPtr(CXType Type)
{
    CXType pointee = clang_Type_getTemplateArgumentAsType(Type, 0);
    return IsReflectedClass(GetClangString(clang_getTypeSpelling(pointee)));
}

bool IsEnum(CXType Type)
{
    return Type.kind == CXTypeKind::CXType_Enum;
}

bool IsNStructOrBuiltin(CXType Type)
{
    static set<string> built_ins = {
        "bool",
        "int8",
        "int16",
        "int32",
        "int64",
        "char",
        "short",
        "int",
        "long",
        "long long",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "unsigned char",
        "unsigned short",
        "unsigned int",
        "unsigned long",
        "unsigned long long",
        "float",
        "double",
        "vec2",
        "vec3",
        "vec4",
        "dvec2",
        "dvec3",
        "dvec4",
        "ivec2",
        "ivec3",
        "ivec4",
        "uvec2",
        "uvec3",
        "uvec4",
        "mat2",
        "mat3",
        "mat4",
        "dmat2",
        "dmat3",
        "dmat4",
        "imat2",
        "imat3",
        "imat4",
        "umat2",
        "umat3",
        "umat4",
        "quat",
        "dquat",
        "std::string",
        "FBinaryBuffer",
        "nilou::FRotator"
    };
    string TypeName = GetClangString(clang_getTypeSpelling(Type));
    if (built_ins.contains(TypeName) || IsReflectedStruct(TypeName) || IsEnum(Type))
        return true;
    return false;
}

bool IsSupportedContainer(CXType Type)
{
    string TypeName = GetClangString(clang_getTypeSpelling(Type));
    std::smatch match;
    if (regex_match(TypeName, match, regex(".*(vector|array|set|map|unordered_map|unordered_set|TAlignedStaticArray)<.+>")))
    {
        if (match[1].str() == "array" || match[1].str() == "TAlignedStaticArray")
        {
            CXType T = clang_Type_getTemplateArgumentAsType(Type, 0);
            if (!IsNStructOrBuiltin(T) && !IsNClassPtr(T) && !IsNClassSmartPtr(T))
                return false;
        }
        else 
        {
            int num = clang_Type_getNumTemplateArguments(Type);
            if (num == 0)
                return false;
            for (int i = 0; i < num; i++)
            {
                CXType T = clang_Type_getTemplateArgumentAsType(Type, i);
                if (!IsNStructOrBuiltin(T) && !IsNClassPtr(T) && !IsNClassSmartPtr(T))
                    return false;
            }
        }
        return true;
    }
    return false;
}

string GetRawType(const string& T)
{
    string raw_T = regex_replace(T, regex("(const |class )|\\*|&"), "");
    raw_T = regex_replace(raw_T, regex(" "), "");
    return raw_T;
}

bool IsSupportedType(CXType Type)
{
    if (IsNStructOrBuiltin(Type))
        return true;
    if (IsNClassPtr(Type))
        return true;
    if (IsNClassSmartPtr(Type))
        return true;
    if (IsSupportedContainer(Type))
        return true;
    return false;
}

bool NeedsReflection(string filepath)
{
    vector<const char*> arguments = {
        "-x",
        "c++-cpp-output"
    };
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        filepath.c_str(), arguments.data(), (int)arguments.size(),
        nullptr, 0,
        CXTranslationUnit_None);
    if (unit == nullptr)
    {
        cerr << "Unable to parse translation unit. Quitting." << endl;
        return false;
    }
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    bool needs_reflection = false;
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data)
        {
            string s = GetCursorSpelling(c);
            if (s == "NCLASS" || s == "NPROPERTY")
            {
                bool* needs_reflection = reinterpret_cast<bool*>(client_data);
                *needs_reflection = true;
                return CXChildVisit_Break;
            }
            return CXChildVisit_Recurse;
        },
        &needs_reflection);
    return needs_reflection;
}

void ParseHeaderFile(string filepath)
{
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        filepath.c_str(), arguments.data(), (int)arguments.size(),
        nullptr, 0,
        CXTranslationUnit_None);
    if (unit == nullptr)
    {
        cerr << "Unable to parse translation unit. Quitting." << endl;
        return;
    }
    pair<vector<CXCursor>, string> data;
    vector<CXCursor>& reflection_classes = data.first;
    data.second = filepath;
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data)
        {
            string s = GetCursorSpelling(c);
            if (clang_getCursorKind(c) == CXCursor_AnnotateAttr) 
            {
                string class_name = fully_qualified(parent);
                if ((s == "reflect-class" || s == "reflect-struct") && !NTypes.contains(class_name)) 
                {
                    pair<vector<CXCursor>, string>* data = reinterpret_cast<pair<vector<CXCursor>, string>*>(client_data);
                    vector<CXCursor>& reflection_classes = data->first;
                    string& current_filepath = data->second;
                    reflection_classes.push_back(parent);
                    NTypes[class_name].Name = class_name;
                    NTypes[class_name].FileName = current_filepath;
                    if (s == "reflect-class")
                        NTypes[class_name].MetaType = "class";
                    else if (s == "reflect-struct")
                        NTypes[class_name].MetaType = "struct";
                }
            }
            
            return CXChildVisit_Recurse;
        },
        &data);

    for (auto& cursor : reflection_classes)
    {
        clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data)
            {
                string cursor_spelling = fully_qualified(c);
                string cursor_kind = GetCursorKindSpelling(c);
                auto cursor_kind_raw = clang_getCursorKind(c);

                if (cursor_kind_raw == CXCursor_Constructor)
                {
                    string class_name = cursor_spelling;
                    string method_name = fully_qualified(parent);
                    string method_args = GetCursorTypeSpelling(parent);
                    if (IsReflectedType(class_name))
                    {
                        vector<string> args;
                        int args_num = clang_Cursor_getNumArguments(c);
                        for (int i = 0; i < args_num; i++)
                        {
                            auto type = GetCursorTypeSpelling(clang_Cursor_getArgument(c, i));
                            args.push_back(type);
                        }
                        NTypes[class_name].Constructors.push_back(args);
                    }
                }
                else if (cursor_kind_raw == CXCursor_AnnotateAttr) 
                {
                    if (cursor_spelling == "reflect-property") 
                    {
                        CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                        string class_name = fully_qualified(class_cursor);
                        string field_name = GetCursorSpelling(parent);
                        string field_type = GetCursorTypeSpelling(parent);
                        if (IsReflectedType(class_name))
                        {
                            if (IsSupportedType(clang_getCursorType(parent)))
                            {
                                auto& Fields = NTypes[class_name].Fields;
                                Fields[field_name] = {true, field_type};
                            }
                            else 
                            {
                                auto& Fields = NTypes[class_name].Fields;
                                cout << std::format("Unsupported type: {} {}, serialization code not generated, but you can still use reflection\n", field_type, field_name);
                                Fields[field_name] = {false, field_type};
                            }
                        }
                    }
                    else if (cursor_spelling == "reflect-method")
                    {
                        CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                        string class_name = fully_qualified(class_cursor);
                        string method_name = GetCursorSpelling(parent);
                        string method_args = GetCursorTypeSpelling(parent);
                        if (IsReflectedType(class_name))
                        {
                            auto& Methods = NTypes[class_name].Methods;
                            Methods.insert(method_name);
                        }
                    }
                    
                }
                else if (cursor_kind_raw == CXCursor_CXXBaseSpecifier) 
                {
                    vector<string> tokens = Split(cursor_spelling, ':');
                    string base_class = cursor_spelling;
                    base_class = regex_replace(base_class, regex("class "), "");
                    string derived_class = fully_qualified(parent);
                    NTypes[base_class].DerivedClasses.insert(derived_class);
                    NTypes[derived_class].BaseClass = GetRawType(base_class);
                }
                
                return CXChildVisit_Recurse;
            },
            nullptr);
    }

    
    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
    
}

static const string& indent = "\t\t";

string GenerateTypeRegistry(const TypeMetaData& NClass)
{
    const auto ClassName = NClass.Name;
    string CtorBody;
    for (auto& Args : NClass.Constructors)
    {
        string args;
        for (int i = 0; i < Args.size(); i++)
        {
            args += ", " + Args[i];
        }
        CtorBody += indent+std::format("Mngr.AddConstructor<{}{}>();\n", ClassName, args);
    }
    string FieldsBody;
    for (auto& [FieldName, FieldType] : NClass.Fields)
    {
        FieldsBody += indent+std::format("Mngr.AddField<&{1}::{0}>(\"{0}\");\n", 
            FieldName, ClassName);
    }
    string MethodsBody;
    for (auto& MethodName : NClass.Methods)
    {
        MethodsBody += indent+std::format("Mngr.AddMethod<&{1}::{0}>(\"{0}\");\n", 
            MethodName, ClassName);
    }
    string ClassHierarchyBody;
    if (NClass.BaseClass != "")
    {
        ClassHierarchyBody += indent+std::format("Mngr.AddBases<{}, {}>();\n", 
            ClassName, NClass.BaseClass);
    }
    return std::format(
R"(
std::unique_ptr<NClass> {0}::StaticClass_ = nullptr;
const NClass *{0}::GetClass() const 
{{ 
    return {0}::StaticClass(); 
}}
const NClass *{0}::StaticClass()
{{
    return {0}::StaticClass_.get();
}}

template<>
struct TClassRegistry<{0}>
{{
    TClassRegistry(const std::string& InName)
    {{
        {0}::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<{0}>();
{1}{2}{3}{4};
        {0}::StaticClass_->Type = Type_of<{0}>;
        {0}::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<{0}>);
    }}

    static TClassRegistry<{0}> Dummy;
}};
TClassRegistry<{0}> Dummy = TClassRegistry<{0}>("{0}");

)", ClassName, CtorBody, FieldsBody, MethodsBody, ClassHierarchyBody, NClass.BaseClass);
}

pair<string, string> GenerateSerializeBody(const string& FieldName, const string& FieldType)
{
    string SerializeBody, DeserializeBody;
    if (IsReflectedStruct(FieldType))
    {
        SerializeBody += std::format(R"(
    {{
        FArchive local_Ar(content["{0}"], Ar);
        this->{0}.Serialize(local_Ar);
    }})", FieldName);
        DeserializeBody += std::format(R"(
    if (content.contains("{0}"))
    {{
        FArchive local_Ar(content["{0}"], Ar);
        this->{0}.Deserialize(local_Ar);
    }})", FieldName);
    }
    else 
    {
        SerializeBody += std::format(R"(
    {{
        FArchive local_Ar(content["{0}"], Ar);
        TStaticSerializer<decltype(this->{0})>::Serialize(this->{0}, local_Ar);
    }})", FieldName, FieldType);
        DeserializeBody += std::format(R"(
    if (content.contains("{0}"))
    {{
        FArchive local_Ar(content["{0}"], Ar);
        TStaticSerializer<decltype(this->{0})>::Deserialize(this->{0}, local_Ar);
    }})", FieldName);
    }
    return { SerializeBody, DeserializeBody };
}

string GenerateClassSerialize(const TypeMetaData& NClass)
{
    const auto ClassName = NClass.Name;

    string SerializeBody, DeserializeBody;
    for (auto& [FieldName, FieldType] : NClass.Fields)
    {
        if (FieldType.first)
        {
            auto [serialize_body, deserialize_body] = GenerateSerializeBody(FieldName, FieldType.second);
            SerializeBody += serialize_body;
            DeserializeBody += deserialize_body;
        }
    }

    string BaseSerialize, BaseDeserialize;
    if (NClass.BaseClass != "")
    {
        BaseSerialize = std::format("{}::Serialize(Ar);", NClass.BaseClass);
        BaseDeserialize = std::format("{}::Deserialize(Ar);", NClass.BaseClass);
    }

    if (NClass.MetaType == "class")
    {
        return std::format(
R"(
void {0}::Serialize(FArchive& Ar)
{{
    {1}
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "{0}";
    nlohmann::json &content = Node["Content"];
{2}
    this->bIsSerializing = false;
}}

void {0}::Deserialize(FArchive& Ar)
{{
    {4}
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];
{3}
}}
)", ClassName, BaseSerialize, SerializeBody, DeserializeBody, BaseDeserialize);
    }
    else 
    {
        return std::format(
R"(
void {0}::Serialize(FArchive& Ar)
{{
    {1}
    nlohmann::json &content = Ar.Node;
{2}
}}

void {0}::Deserialize(FArchive& Ar)
{{
    nlohmann::json &content = Ar.Node;
{3}
    {4}
}}
)", ClassName, BaseSerialize, SerializeBody, DeserializeBody, BaseDeserialize);
    }
}

void GenerateCode()
{
    for (auto& [ClassName, NClass] : NTypes)
    {
            string TypeRegistry = GenerateTypeRegistry(NClass);
            string Serialize = GenerateClassSerialize(NClass);
            NClass.GeneratedFileCode = std::format(R"(#include "{}"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;
{}
{})", NClass.FileName, TypeRegistry, Serialize);
    }
}

void WriteCode(string GeneratedCodePath)
{
    for (auto& [ClassName, NClass] : NTypes)
    {
        auto tokens = Split(ClassName, ':');
        string raw_class_name = tokens[tokens.size()-1];
        ofstream out_stream(GeneratedCodePath + "/" + raw_class_name + ".generated.cpp", ios::out);
        out_stream << NClass.GeneratedFileCode;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: HeaderTool <src directory> <generated code directory> <include path 0> <include path 1> ..." << endl;
        return -1;
    }
    std::string DirectoryName = argv[1];
    //std::string DirectoryName = "./src"; //argv[1];
    if (DirectoryName[DirectoryName.size()-1] == '\\' || DirectoryName[DirectoryName.size()-1] == '/')
        DirectoryName = DirectoryName.substr(0, DirectoryName.size()-1);
    std::string GeneratedCodePath = argv[2];
    //std::string GeneratedCodePath = "./src/Runtime/Generated"; //argv[2];
    if (GeneratedCodePath[GeneratedCodePath.size()-1] == '\\' || GeneratedCodePath[GeneratedCodePath.size()-1] == '/')
        GeneratedCodePath = GeneratedCodePath.substr(0, GeneratedCodePath.size()-1);

    map<string, long long> CachedHeaderModifiedTime;
    filesystem::path CachedHeaderModifiedTimePath = filesystem::path(GeneratedCodePath) / filesystem::path("CachedHeaderModifiedTime.txt");
    if (!filesystem::exists(GeneratedCodePath))
    {
        filesystem::create_directories(GeneratedCodePath);
    }
    if (filesystem::exists(CachedHeaderModifiedTimePath))
    {
        ifstream in{CachedHeaderModifiedTimePath.string()};
        while (!in.eof())
        {
            string filename;
            long long last_modified_time;
            in >> filename >> last_modified_time;
            CachedHeaderModifiedTime[filename] = last_modified_time;
        }
    }

    for (int i = 3; i < argc; i++)
    {
        arguments.push_back("-I");
        arguments.push_back(argv[i]);
    }

    std::string ImplementationBody;

    std::string MarkedClassesEnumBody;

    std::string IncludedPaths;

    std::vector<string> ClassNames;

    std::vector<string> files;

    Directory dir(DirectoryName);
    dir.ForEachFile(true, 
    [&](const std::string& filepath) 
        {
            if ((EndsWith(filepath, ".h") || EndsWith(filepath, ".hpp")) && 
                NeedsReflection(filepath))
            {
                long long cached_last_modified_time = CachedHeaderModifiedTime[filesystem::path(filepath).generic_string()];
                long long last_modified_time = filesystem::last_write_time(filepath).time_since_epoch().count();
            
                if (cached_last_modified_time == 0 || last_modified_time != cached_last_modified_time)
                {
                    files.push_back(filepath);
                    CachedHeaderModifiedTime[filesystem::path(filepath).generic_string()] = last_modified_time;
                }
            }
        });

    if (files.size() > 0)
    {
        mutex cout_mutex;
        std::for_each(std::execution::par, files.begin(), files.end(), 
            [&cout_mutex](const string& filepath) {
                std::unique_lock<mutex> lock(cout_mutex);
                cout << filepath << endl;
                lock.unlock();
                ParseHeaderFile(filepath);
            });
        GenerateCode();
        WriteCode(GeneratedCodePath);

        ofstream out{CachedHeaderModifiedTimePath};
        int i = 0;
        for (auto& [filename, last_modified_time] : CachedHeaderModifiedTime)
        {
            out << filename << " " << last_modified_time;
            i++;
            if (i != CachedHeaderModifiedTime.size())
                out << "\n";
        }
    }
    else
    {
        cout << "[NilouHeaderTool] All header files are up-to-date.\n";
    }

    return 0;
}