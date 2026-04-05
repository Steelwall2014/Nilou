#pragma once
#include "Templates/TypeTraits.h"

#ifdef __clang__
#define NCLASS __attribute__((annotate("reflect-class")))
#else
#define NCLASS
#endif

#ifdef __clang__
#define NSTRUCT __attribute__((annotate("reflect-struct")))
#else
#define NSTRUCT
#endif

#ifdef __clang__
#define NENUM __attribute__((annotate("reflect-enum")))
#else
#define NENUM
#endif

#ifdef __clang__
#define NPROPERTY() __attribute__((annotate("reflect-property")))
#else
#define NPROPERTY()
#endif

#ifdef __clang__
#define NFUNCTION() __attribute__((annotate("reflect-method")))
#else
#define NFUNCTION()
#endif

#define GENERATED_BODY() \
    private: \
        template<typename T>  \
        friend class TClassRegistry; \
        static NClass* Z_StaticClass; \
    public: \
        static NClass *StaticClass() { return Z_StaticClass; }

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

#define DECLARE_VERTEX_FACTORY_TYPE(FactoryClass) \
    public: \
        static FVertexFactoryType StaticType; \
        virtual FVertexFactoryType* GetType() const override; \

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
        virtual FShaderType* GetType() const override; \

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

#define IMPLEMENT_SHADER_TYPE(ShaderClass, ShaderFilename, EntryPointName, ShaderFrequency) \
	FShaderType ShaderClass::StaticType( \
		#ShaderClass, \
		ShaderFilename, \
		EntryPointName, \
		ShaderFrequency, \
		TIsDerivedFrom<ShaderClass, FGlobalShader>::Value ? EShaderMetaType::Global : EShaderMetaType::Material, \
		ShaderClass::ShouldCompilePermutation, \
		ShaderClass::ModifyCompilationEnvironment, \
		ShaderClass::FPermutationDomain::PermutationCount \
		); \
	FShaderType* ShaderClass::GetType() const \
	{ \
		TShaderFrequencyAssertHelper<ShaderFrequency, TIsDerivedFrom<ShaderClass, FMaterialShader>::Value>(); \
		return &StaticType; \
	}

/** 
 * Declares a graphics pipeline type as a class, binding it to a specific VS and PS.
 * Use in a header file. Override ShouldCompilePermutation / ModifyCompilationEnvironment as needed.
 * 
 * Usage:
 *   DECLARE_GRAPHICS_PIPELINE(FMyPipeline, FMyVertexShader, FMyPixelShader)
 *
 * The class exposes:
 *   using VertexShaderType = FMyVertexShader;
 *   using PixelShaderType  = FMyPixelShader;
 *   static FGraphicsPipeline StaticType;
 *   using FPermutationDomain = ...;        -- defaults to FShaderPermutationNone
 */
#define DECLARE_GRAPHICS_PIPELINE(PipelineClass, VertexShaderClass, PixelShaderClass) \
    class PipelineClass \
    { \
    public: \
        using VertexShaderType = VertexShaderClass; \
        using PixelShaderType  = PixelShaderClass; \
        static FGraphicsPipeline StaticType; \
        using FPermutationDomain = FShaderPermutationNone; \
        static bool ShouldCompilePermutation(const FGraphicsPipelinePermutationParameters&) { return true; } \
        static void ModifyCompilationEnvironment(const FGraphicsPipelinePermutationParameters&, FShaderCompilerEnvironment&) {} \
    };

/**
 * Defines (registers) a graphics pipeline type declared with DECLARE_GRAPHICS_PIPELINE.
 * Place in a .cpp file.
 *
 * Usage:
 *   IMPLEMENT_GRAPHICS_PIPELINE(FMyPipeline)
 */
#define IMPLEMENT_GRAPHICS_PIPELINE(PipelineClass) \
    FGraphicsPipeline PipelineClass::StaticType( \
        #PipelineClass, \
        &PipelineClass::VertexShaderType::StaticType, \
        &PipelineClass::PixelShaderType::StaticType, \
        PipelineClass::ShouldCompilePermutation, \
        PipelineClass::ModifyCompilationEnvironment, \
        PipelineClass::FPermutationDomain::PermutationCount \
    );

#define DEFINE_GRAPHICS_PIPELINE(PipelineClass, VertexShaderClass, PixelShaderClass) \
	DECLARE_GRAPHICS_PIPELINE(PipelineClass, VertexShaderClass, PixelShaderClass); \
	IMPLEMENT_GRAPHICS_PIPELINE(PipelineClass); \
