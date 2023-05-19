#pragma once

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
        friend class ::TClassRegistry; \
        static std::unique_ptr<::NClass> StaticClass_; \
    public: \
        virtual const ::NClass *GetClass() const; \
        static const ::NClass *StaticClass(); \
        virtual void Serialize(FArchive& Ar); \
        virtual void Deserialize(FArchive& Ar);

#define GENERATED_STRUCT_BODY() \
    private: \
        template<typename T>  \
        friend class ::TClassRegistry; \
        static std::unique_ptr<::NClass> StaticClass_; \
    public: \
        const ::NClass *GetClass() const; \
        static const ::NClass *StaticClass(); \
        void Serialize(FArchive& Ar); \
        void Deserialize(FArchive& Ar);