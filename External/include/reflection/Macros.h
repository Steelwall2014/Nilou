#pragma once

#ifdef __clang__
#define NCLASS __attribute__((annotate("reflect-class")))
#else
#define NCLASS
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

#define GENERATE_BODY() \
    private: \
        template<typename T>  \
        friend class reflection::TClassRegistry; \
        static std::unique_ptr<reflection::NClass> StaticClass_; \
    public: \
        virtual const reflection::NClass *GetClass() const; \
        static const reflection::NClass *StaticClass();