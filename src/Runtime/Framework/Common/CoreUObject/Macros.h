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
