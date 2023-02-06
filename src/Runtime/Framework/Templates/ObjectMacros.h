#pragma once
#include "TypeTraits.h"

#define UCLASS()

#define DEFINE_DYNAMIC_DATA(Type, Name) \
	public: \
		/** Updates the component's DynamicData property, and tells it to refresh */ \
		inline void Set##Name(const Type &In##Name) { Name = In##Name; MarkRenderDynamicDataDirty(); } \
		/** Get the component's property */ \
		inline Type Get##Name() const { return Name; } \
	protected: \
		Type Name;

// #define DEFINE_RENDER_STATE_DATA(Type, Name) \
// 	public: \
// 		/** Updates the component's RenderState property, and tells it to refresh */ \
// 		inline void Set##Name(const Type &In##Name) { Name = In##Name; MarkRenderStateDirty(); } \
// 		/** Get the component's property */ \
// 		inline Type Get##Name() const { return Name; } \
// 	private: \
// 		Type Name;

#define GENERATE_CLASS_INFO() \
    public: \
        virtual std::string GetClassName(); \
        virtual EUClasses GetClassEnum(); \
        virtual const UClass *GetClass(); \
        static const UClass *StaticClass();

#define DECLARE_VERTEX_FACTORY_TYPE(FactoryClass) \
    public: \
        static FVertexFactoryType StaticType; \
        virtual FVertexFactoryType* GetType() const override;

#define IMPLEMENT_VERTEX_FACTORY_TYPE(FactoryClass, ShaderFilename) \
	FVertexFactoryType FactoryClass::StaticType( \
		#FactoryClass, \
		ShaderFilename, \
		FactoryClass::ShouldCompilePermutation, \
		FactoryClass::ModifyCompilationEnvironment, \
		FactoryClass::FPermutationDomain::PermutationCount \
		); \
		FVertexFactoryType* FactoryClass::GetType() const { return &StaticType; }

#define DECLARE_SHADER_TYPE() \
    public: \
        static FShaderType StaticType; \
        virtual FShaderType* GetType() const override;

#define DECLARE_GLOBAL_SHADER(ShaderClass) \
	class ShaderClass : public FGlobalShader\
	{ \
	public: \
		DECLARE_SHADER_TYPE() \
	};

#define DECLARE_MATERIAL_SHADER(ShaderClass) \
	class ShaderClass : public FMaterialShader\
	{ \
	public: \
		DECLARE_SHADER_TYPE() \
	};

#define IMPLEMENT_SHADER_TYPE(ShaderClass, ShaderFilename, ShaderFrequency, ShaderMetaType) \
	FShaderType ShaderClass::StaticType( \
		#ShaderClass, \
		ShaderFilename, \
		ShaderFrequency, \
		EShaderMetaType::SMT_##ShaderMetaType, \
		ShaderClass::ShouldCompilePermutation, \
		ShaderClass::ModifyCompilationEnvironment, \
		ShaderClass::FPermutationDomain::PermutationCount \
		); \
	FShaderType* ShaderClass::GetType() const \
	{ \
		TShaderFrequencyAssertHelper<ShaderFrequency, TIsDerivedFrom<ShaderClass, FMaterialShader>::Value>(); \
		return &StaticType; \
	}

#define DECLARE_MATERIAL_TYPE() \
    public: \
        static FMaterialType StaticType; \
        virtual FMaterialType* GetType() const override; \

#define IMPLEMENT_MATERIAL_TYPE(MaterialClass, ShaderFilename) \
	FMaterialType MaterialClass::StaticType( \
		#MaterialClass, \
		ShaderFilename, \
		MaterialClass::ShouldCompilePermutation, \
		MaterialClass::ModifyCompilationEnvironment, \
		MaterialClass::FPermutationDomain::PermutationCount \
		); \
	FMaterialType* MaterialClass::GetType() const { return &StaticType; }

#define IMPLEMENT_MODULE(ModuleName) \
	ModuleName::ModuleName() \
	{ \
		GetModuleManager()->AddModule(#ModuleName, this); \
	} \
	ModuleName *G##ModuleName = new ModuleName;