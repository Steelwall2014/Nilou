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
    CXCursor Cursor;
    string FileName;
    string Name;
    string BaseClass;
    set<string> DerivedClasses;
    map<string, string> Fields;
    set<string> Methods;
    // vector<vector<string>> Constructors;
    string GeneratedCode;
    string MetaType; // class or struct
    vector<string> EnumValues;
    vector<string> AdditionalIncludes;
};
map<string, TypeMetaData> NTypes;

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

string StripClassStructPrefix(const string& T)
{
    string raw_T = regex_replace(T, regex("(class |struct )"), "");
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
        CXTranslationUnit_Incomplete);
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
    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
    return needs_reflection;
}

void FixIncompleteForwardDeclarations(TypeMetaData& CurrentType, CXCursor FieldCursor)
{
    ClangVisitChildren(FieldCursor, [&](CXCursor c, CXCursor parent)
    {
        string s = GetCursorSpelling(c);
        if (c.kind == CXCursor_TypeRef)
        {
            s = RemoveNamespace(StripClassStructPrefix(s));
            if (IsReflectedType(s))
            {
                auto& AdditionalIncludes = CurrentType.AdditionalIncludes;
                auto Found = std::find(AdditionalIncludes.begin(), AdditionalIncludes.end(), NTypes[s].FileName);
                if (Found == AdditionalIncludes.end() && NTypes[s].FileName != CurrentType.FileName)
                {
                    CurrentType.AdditionalIncludes.push_back(NTypes[s].FileName);
                }
            }
        }
        
        return CXChildVisit_Recurse;
    });
}

bool ParseHeaderFile(const std::set<string>& filepaths, const std::vector<const char*>& arguments)
{
    bool bAllSucceeded = true;
    std::vector<CXIndex> Indices;
    std::map<string, CXTranslationUnit> TranslationUnits;
    mutex TranslationUnitsMutex;
    std::for_each(std::execution::par, filepaths.begin(), filepaths.end(), [&](const string& filepath) 
    {
        CXIndex index = clang_createIndex(0, 0);
        CXTranslationUnit unit = nullptr;
        CXErrorCode error = clang_parseTranslationUnit2(
            index,
            filepath.c_str(), arguments.data(), arguments.size(),
            nullptr, 0,
            CXTranslationUnit_None,
            &unit);

        std::lock_guard<mutex> lock(TranslationUnitsMutex);
        Indices.push_back(index);
        if (error == CXError_Success)
        {
            TranslationUnits[filepath] = unit;
        }
        if (error != CXError_Success) 
        {
            string error_str;
            switch (error)
            {
                case CXError_Failure:
                    error_str = "CXError_Failure";
                    break;
                case CXError_Crashed:
                    error_str = "CXError_Crashed";
                    break;
                case CXError_InvalidArguments:
                    error_str = "CXError_InvalidArguments";
                    break;
                case CXError_ASTReadError:
                    error_str = "CXError_ASTReadError";
                    break;
                default:
                    break;
            };
            cerr << error_str << " occurred while parsing file: " << filepath << endl;
            bAllSucceeded = false;
        }
    });
    if (!bAllSucceeded)
    {
        return false;
    }

    // Intrinsic types
    NTypes["NObject"].Name = "NObject";
    NTypes["NObject"].MetaType = "class";
    NTypes["NPackage"].Name = "NPackage";
    NTypes["NPackage"].MetaType = "class";
    NTypes["NClass"].Name = "NClass";
    NTypes["NClass"].MetaType = "class";

    mutex NTypesMutex;
    std::for_each(std::execution::par, TranslationUnits.begin(), TranslationUnits.end(), [&](auto& pair) 
    {
        auto& [filepath, unit] = pair;
        CXCursor cursor = clang_getTranslationUnitCursor(unit);
        ClangVisitChildren(cursor, [&](CXCursor c, CXCursor parent)
        {
            auto cursor_kind = clang_getCursorKind(c);
            if (cursor_kind == CXCursor_AnnotateAttr) 
            {
                std::lock_guard<std::mutex> lock(NTypesMutex);
                string annotation = GetCursorSpelling(c);
                string class_name = GetCursorSpelling(parent);
                if ((annotation == "reflect-class" || annotation == "reflect-struct" || annotation == "reflect-enum") && !NTypes.contains(class_name)) 
                {
                    NTypes[class_name].Cursor = parent;
                    NTypes[class_name].Name = class_name;
                    NTypes[class_name].FileName = filepath;
                    if (annotation == "reflect-class")
                        NTypes[class_name].MetaType = "class";
                    else if (annotation == "reflect-struct")
                        NTypes[class_name].MetaType = "struct";
                    else if (annotation == "reflect-enum")
                        NTypes[class_name].MetaType = "enum";
                }
            }
            
            return CXChildVisit_Recurse;
        });
    });

    std::for_each(std::execution::par, TranslationUnits.begin(), TranslationUnits.end(), [&](auto& pair) 
    {
        auto& [filepath, unit] = pair;
        CXCursor cursor = clang_getTranslationUnitCursor(unit);
        ClangVisitChildren(cursor, [&](CXCursor c, CXCursor parent)
        {
            auto cursor_kind = clang_getCursorKind(c);
            if (cursor_kind == CXCursor_CXXBaseSpecifier) 
            {
                std::lock_guard<std::mutex> lock(NTypesMutex);
                string cursor_spelling = GetCursorSpelling(c);
                string base_class = RemoveNamespace(StripClassStructPrefix(cursor_spelling));
                string derived_class = GetCursorSpelling(parent);
                if (IsReflectedType(base_class) && IsReflectedType(derived_class))
                {
                    NTypes[base_class].DerivedClasses.insert(derived_class);
                    NTypes[derived_class].BaseClass = base_class;
                }
            }
            
            return CXChildVisit_Recurse;
        });
    });

    
    std::for_each(std::execution::par, NTypes.begin(), NTypes.end(), [&](auto& pair) 
    {
        auto& [name, CurrentType] = pair;
        ClangVisitChildren(CurrentType.Cursor, [&](CXCursor c, CXCursor parent)
        {
            string cursor_spelling = GetCursorSpelling(c);
            string cursor_kind_str = GetCursorKindSpelling(c);
            auto cursor_kind = clang_getCursorKind(c);

            if (cursor_kind == CXCursor_Constructor)
            {
                // string class_name = cursor_spelling;
                // string method_name = GetCursorSpelling(parent);
                // string method_args = GetCursorTypeSpelling(parent);
                // if (IsReflectedType(class_name))
                // {
                //     vector<string> args;
                //     int args_num = clang_Cursor_getNumArguments(c);
                //     for (int i = 0; i < args_num; i++)
                //     {
                //         auto type = GetCursorTypeSpelling(clang_Cursor_getArgument(c, i));
                //         args.push_back(type);
                //     }
                //     NTypes[class_name].Constructors.push_back(args);
                // }
            }
            else if (cursor_kind == CXCursor_AnnotateAttr) 
            {
                if (cursor_spelling == "reflect-property") 
                {
                    CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                    string class_name = GetCursorSpelling(class_cursor);
                    if (class_name == CurrentType.Name)
                    {
                        string field_name = GetCursorSpelling(parent);
                        string field_type = GetCursorTypeSpelling(parent);
                        CurrentType.Fields[field_name] = field_type;
                        FixIncompleteForwardDeclarations(CurrentType, parent);
                    }
                }
                else if (cursor_spelling == "reflect-method")
                {
                    CXCursor class_cursor = clang_getCursorSemanticParent(parent);
                    string class_name = GetCursorSpelling(class_cursor);
                    if (class_name == CurrentType.Name)
                    {
                        string method_name = GetCursorSpelling(parent);
                        string method_args = GetCursorTypeSpelling(parent);
                        CurrentType.Methods.insert(method_name);
                    }
                }
                
            }
            else if (cursor_kind == CXCursor_EnumConstantDecl)
            {
                string enum_name = GetCursorSpelling(parent);
                string enum_value = GetCursorSpelling(c);
                if (enum_name == CurrentType.Name)
                {
                    CurrentType.EnumValues.push_back(enum_value);
                }
            }
            
            return CXChildVisit_Recurse;
        });
    });
    
    for (auto& [filepath, unit] : TranslationUnits)
    {
        clang_disposeTranslationUnit(unit);
    }
    for (auto& index : Indices)
    {
        clang_disposeIndex(index);
    }
    return true;
}

string GenerateIncludes(const TypeMetaData& Type)
{
    string Includes;
    Includes += std::format("#include \"NObject/NilouType.h\"\n");
    Includes += std::format("#include \"{}\"\n", Type.FileName);
    for (auto& Include : Type.AdditionalIncludes)
    {
        Includes += std::format("#include \"{}\"\n", Include);
    }
    return Includes;
}

string GenerateTypeRegistry(const TypeMetaData& Type)
{
    const auto ClassName = Type.Name;
    string BeginClassRegistry;
    string RegistryBody;
    string EndClassRegistry;
    if (Type.MetaType == "enum")
    {
        BeginClassRegistry += std::format(
            "BEGIN_ENUM_REGISTRY({}, EClassFlags::Native)\n", ClassName);
        for (auto& EnumValue : Type.EnumValues)
        {
            RegistryBody += std::format("\tENUM_VALUE({});\n", EnumValue);
        }
        EndClassRegistry = std::format("END_ENUM_REGISTRY({})\n", ClassName);
    }
    else if (Type.MetaType == "struct")
    {
        BeginClassRegistry += std::format(
            "BEGIN_STRUCT_REGISTRY({}, {}, EClassFlags::Native)\n", 
            ClassName, 
            Type.BaseClass!="" ? Type.BaseClass : "NullSuperClass");
        for (auto& [FieldName, FieldType] : Type.Fields)
        {
            RegistryBody += std::format("\tSTRUCT_PROPERTY({})\n", FieldName);
        }
        EndClassRegistry = std::format("END_STRUCT_REGISTRY({})\n", ClassName);
    }
    else if (Type.MetaType == "class")
    {
        BeginClassRegistry += std::format(
            "BEGIN_CLASS_REGISTRY({}, {}, EClassFlags::Native)\n", 
            ClassName, 
            Type.BaseClass!="" ? Type.BaseClass : "NullSuperClass");
        for (auto& [FieldName, FieldType] : Type.Fields)
        {
            RegistryBody += std::format("\tCLASS_PROPERTY({})\n", FieldName);
        }
        EndClassRegistry = std::format("END_CLASS_REGISTRY({})\n", ClassName);
    }
    return std::format(
                "{0}\n"
                "{1}\n"
                "{2}\n", 
                BeginClassRegistry, RegistryBody, EndClassRegistry);
}

void GenerateCode()
{
    for (auto& [ClassName, Type] : NTypes)
    {
        if (Type.FileName == "") continue;
        string Includes = GenerateIncludes(Type);
        string TypeRegistry = GenerateTypeRegistry(Type);
        Type.GeneratedCode = std::format(
            "{}\n"
            "namespace nilou {{\n"
            "{}\n"
            "}}", 
            Includes, 
            TypeRegistry);
    }
}

void WriteCode(string GeneratedCodePath)
{
    std::set<string> ExpectedTypes;
    for (auto& [ClassName, Type] : NTypes)
    {
        ExpectedTypes.insert(Type.Name);
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
    for (auto& [ClassName, Type] : NTypes)
    {
        if (Type.FileName == "") continue;
        fs::path filepath = GeneratedCodePath + "/" + Type.Name + ".gen.cpp";
        if (fs::exists(filepath))
        {
            ifstream in_stream(filepath);
            string content((std::istreambuf_iterator<char>(in_stream)), std::istreambuf_iterator<char>());
            if (content == Type.GeneratedCode)
            {
                continue;
            }
        }
        ofstream out_stream(filepath, ios::out);
        out_stream << Type.GeneratedCode;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cout << "Usage: NilouHeaderTool -InputDirectory=<directory> -OutputDirectory=<directory> [clang arguments...]" << endl;
        return -1;
    }
    std::string InputDirectory;
    std::string OutputDirectory;
    std::vector<const char*> ClangArguments;

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
        else
        {
            ClangArguments.push_back(argv[i]);
        }
    }

    map<string, long long> CachedHeaderModifiedTime;
    fs::path CachedHeaderModifiedTimePath = fs::path(OutputDirectory) / fs::path("CachedHeaderModifiedTime.txt");
    if (!fs::exists(OutputDirectory))
    {
        fs::create_directories(OutputDirectory);
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

    std::set<string> FilesThatNeedsReflection;
    bool bHasChangedFiles = false;

    {
        fs::path ExePath = fs::canonical(argv[0]);
        std::string ExePathString = ExePath.generic_string();
        long long ExeModifiedTime = fs::last_write_time(ExePath).time_since_epoch().count();
        if (CachedHeaderModifiedTime.find(ExePathString) == CachedHeaderModifiedTime.end() ||
            CachedHeaderModifiedTime[ExePathString] != ExeModifiedTime)
        {
            bHasChangedFiles = true;
            CachedHeaderModifiedTime[ExePathString] = ExeModifiedTime;
        }
    }

    ForEachFile(InputDirectory, true, 
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
        for (auto& FileName : FilesThatNeedsReflection)
        {
            cout << "[NilouHeaderTool] " << FileName << endl;
        }
        if (!ParseHeaderFile(FilesThatNeedsReflection, ClangArguments))
        {
            return -1;
        }
        GenerateCode();
        WriteCode(OutputDirectory);

        ofstream out{CachedHeaderModifiedTimePath};
        for (auto& [filename, last_modified_time] : CachedHeaderModifiedTime)
        {
            out << filename << " " << last_modified_time << "\n";
        }
    }
    else
    {
        cout << "[NilouHeaderTool] All header files are up-to-date." << endl;
    }

    return 0;
}