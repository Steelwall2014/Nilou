#include <cassert>
#include <fstream>
#include <functional>
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
namespace fs = std::filesystem;

template <typename Func>
void ForEachFile(const std::string &DirectoryName, bool bFindInChildren, Func&& InFunc)
{
    if (!fs::exists(DirectoryName))
    {
        std::cout << "Directory: " + DirectoryName + " doesn't exist" << std::endl;
        return;
    }
        
    for (const fs::directory_entry & dir_entry : 
        fs::recursive_directory_iterator(DirectoryName))
    {
        if (!dir_entry.is_directory())
        {
            std::string filepath = dir_entry.path().generic_string();
            InFunc(filepath);
        }
    }
}

ostream& operator<<(ostream& stream, const CXString& str)
{
    const char* s = clang_getCString(str);
    stream << s;
    clang_disposeString(str);
    return stream;
}

struct TypeMetaData
{
    string FileName;
    string Name;
    string NameWithoutNamespace;
    string BaseClass;
    set<string> DerivedClasses;
    map<string, string> Fields;
    set<string> Methods;
    vector<vector<string>> Constructors;
    string GeneratedFileCode;
    string MetaType; // class or struct
    vector<string> EnumValues;
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

bool IsReflectedEnum(const string& TypeName)
{
    return NTypes.contains(TypeName) && NTypes[TypeName].MetaType == "enum";
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
    if (regex_match(TypeName, match, regex(".*(vector|array|set|map|unordered_map|unordered_set|TAlignedStaticArray|TArray|TMap|TSet)<.+>")))
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

void ClangVisitChildren(CXCursor cursor, std::function<CXChildVisitResult(CXCursor, CXCursor)> callback)
{
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data)
        {
            auto* callback = reinterpret_cast<std::function<CXChildVisitResult(CXCursor, CXCursor)>*>(client_data);
            return (*callback)(c, parent);
        },
        &callback);
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
    ClangVisitChildren(
        cursor,
        [&needs_reflection](CXCursor c, CXCursor parent)
        {
            string s = GetCursorSpelling(c);
            if (s == "NCLASS" || s == "NPROPERTY")
            {
                needs_reflection = true;
                return CXChildVisit_Break;
            }
            return CXChildVisit_Recurse;
        });
    return needs_reflection;
}

void ParseHeaderFile(string filepath, std::vector<string> IncludePaths)
{
    std::vector<const char*> arguments = {
        "-x",
        "c++",
        "-std=c++20",
        "-D __clang__",
        "-D __META_PARSER__"
    };
    for (auto& path : IncludePaths)
    {
        arguments.push_back("-I");
        arguments.push_back(path.c_str());
    }
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
    vector<CXCursor> reflection_classes;
    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    ClangVisitChildren(
        cursor,
        [&](CXCursor c, CXCursor parent)
        {
            string s = GetCursorSpelling(c);
            if (clang_getCursorKind(c) == CXCursor_AnnotateAttr) 
            {
                string class_name = fully_qualified(parent);
                if ((s == "reflect-class" || s == "reflect-struct" || s == "reflect-enum") && !NTypes.contains(class_name)) 
                {
                    reflection_classes.push_back(parent);
                    NTypes[class_name].Name = class_name;
                    NTypes[class_name].NameWithoutNamespace = RemoveNamespace(class_name);
                    NTypes[class_name].FileName = filepath;
                    if (s == "reflect-class")
                        NTypes[class_name].MetaType = "class";
                    else if (s == "reflect-struct")
                        NTypes[class_name].MetaType = "struct";
                    else if (s == "reflect-enum")
                        NTypes[class_name].MetaType = "enum";
                }
            }
            
            return CXChildVisit_Recurse;
        });

    for (auto& cursor : reflection_classes)
    {
        ClangVisitChildren(
            cursor,
            [&](CXCursor c, CXCursor parent)
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
                            auto& Fields = NTypes[class_name].Fields;
                            Fields[field_name] = field_type;
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
                else if (cursor_kind_raw == CXCursor_EnumConstantDecl)
                {
                    string enum_name = fully_qualified(parent);
                    string enum_value = GetCursorSpelling(c);
                    if (IsReflectedType(enum_name))
                    {
                        auto& EnumValues = NTypes[enum_name].EnumValues;
                        EnumValues.push_back(enum_value);
                    }
                }
                
                return CXChildVisit_Recurse;
            });
    }

    
    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
    
}

string GenerateTypeRegistry(const TypeMetaData& NClass)
{
    const auto ClassName = NClass.Name;
    const auto ClassNameWithoutNamespace = NClass.NameWithoutNamespace;
    string BeginClassRegistry;
    string RegistryBody;
    string EndClassRegistry;
    if (NClass.MetaType == "enum")
    {
        BeginClassRegistry += std::format(
            "BEGIN_ENUM_REGISTRY({}, EClassFlags::Native)\n", ClassNameWithoutNamespace);
        for (auto& EnumValue : NClass.EnumValues)
        {
            RegistryBody += std::format("\tENUM_VALUE({});\n", EnumValue);
        }
        EndClassRegistry = std::format("END_ENUM_REGISTRY({})\n", ClassNameWithoutNamespace);
    }
    else 
    {
        BeginClassRegistry += std::format(
            "BEGIN_CLASS_REGISTRY({}, {}, {}, EClassFlags::Native)\n", 
            IsReflectedClass(ClassName) ? "Object" : IsReflectedEnum(ClassName) ? "Enum" : "Struct", 
            ClassNameWithoutNamespace, 
            NClass.BaseClass!="" ? NClass.BaseClass : "NullSuperClass");
        for (auto& [FieldName, FieldType] : NClass.Fields)
        {
            RegistryBody += std::format("\tCLASS_PROPERTY({})\n", FieldName);
        }
        EndClassRegistry = std::format("END_CLASS_REGISTRY({})\n", ClassNameWithoutNamespace);
    }
    return std::format(
                "{0}\n"
                "{1}\n"
                "{2}\n", 
                BeginClassRegistry, RegistryBody, EndClassRegistry);
}

void GenerateCode()
{
    for (auto& [ClassName, NClass] : NTypes)
    {
        string TypeRegistry = GenerateTypeRegistry(NClass);
        NClass.GeneratedFileCode = std::format(
            "#include \"{}\"\nnamespace nilou {{\n{}\n}}", NClass.FileName, TypeRegistry);
    }
}

void WriteCode(string GeneratedCodePath)
{
    std::set<string> ExpectedTypes;
    for (auto& [ClassName, NClass] : NTypes)
    {
        ExpectedTypes.insert(NClass.NameWithoutNamespace);
    }
    ForEachFile(GeneratedCodePath, false, 
        [&](const std::string& filepath)
        {
            if (EndsWith(filepath, ".gen.cpp"))
            {
                string filename = fs::path(filepath).filename().string();
                string NameWithoutNamespace = filename.substr(0, filename.size()-8);    // 8 = ".gen.cpp".size()
                if (ExpectedTypes.find(NameWithoutNamespace) == ExpectedTypes.end())
                {
                    fs::remove(filepath);
                }
            }
        });
    for (auto& [ClassName, NClass] : NTypes)
    {
        if (NClass.FileName == "") continue;
        fs::path filepath = GeneratedCodePath + "/" + NClass.NameWithoutNamespace + ".gen.cpp";
        if (fs::exists(filepath))
        {
            ifstream in_stream(filepath);
            string content((std::istreambuf_iterator<char>(in_stream)), std::istreambuf_iterator<char>());
            if (content == NClass.GeneratedFileCode)
            {
                continue;
            }
        }
        ofstream out_stream(filepath, ios::out);
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
    fs::path CachedHeaderModifiedTimePath = fs::path(GeneratedCodePath) / fs::path("CachedHeaderModifiedTime.txt");
    if (!fs::exists(GeneratedCodePath))
    {
        fs::create_directories(GeneratedCodePath);
    }
    if (fs::exists(CachedHeaderModifiedTimePath))
    {
        ifstream in{CachedHeaderModifiedTimePath.string()};
        while (!in.eof())
        {
            string filename;
            long long last_modified_time;
            in >> filename >> last_modified_time;
            if (filename != "")
            {
                CachedHeaderModifiedTime[filename] = last_modified_time;
            }
        }
    }

    std::vector<string> IncludedPaths;
    for (int i = 3; i < argc; i++)
    {
        IncludedPaths.push_back(argv[i]);
    }

    std::set<string> FilesThatNeedsReflection;
    bool bHasChangedFiles = false;
    ForEachFile(DirectoryName, true, 
        [&](const std::string& filepath) 
        {
            if ((EndsWith(filepath, ".h") || EndsWith(filepath, ".hpp")) && 
                NeedsReflection(filepath))
            {
                string path = fs::path(filepath).generic_string();
                string filename = fs::path(filepath).filename().replace_extension("").string();
                long long cached_last_modified_time = CachedHeaderModifiedTime[path];
                long long last_modified_time = fs::last_write_time(filepath).time_since_epoch().count();
                FilesThatNeedsReflection.insert(path);
            
                if (cached_last_modified_time == 0 || last_modified_time != cached_last_modified_time)
                {
                    bHasChangedFiles = true;
                    CachedHeaderModifiedTime[path] = last_modified_time;
                }
            }
        });

    if (bHasChangedFiles)
    {
        mutex cout_mutex;
        std::for_each(std::execution::par, FilesThatNeedsReflection.begin(), FilesThatNeedsReflection.end(), 
            [&](const string& filepath) {
                std::unique_lock<mutex> lock(cout_mutex);
                cout << filepath << endl;
                lock.unlock();
                ParseHeaderFile(filepath, IncludedPaths);
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