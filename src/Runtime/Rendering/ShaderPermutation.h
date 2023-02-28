#pragma once
#include <vector>

#include "ShaderCompileEnvironment.h"
#include "Common/AssertionMacros.h"

namespace nilou {
    struct FShaderPermutationBool
    {
        using Type = bool;
        static constexpr bool IsMultiDimensional = false;
        static constexpr int32 PermutationCount = 2;
        static int32 ToDimensionValueId(Type E)
        {
            return E ? 1 : 0;
        }
        static bool ToDefineValue(Type E)
        {
            return E;
        }
        static Type FromDimensionValueId(int32 PermutationId)
        {
            check(PermutationId == 0 || PermutationId == 1);
            return PermutationId == 1;
        }
        static std::vector<bool> GetValueRange()
        {
            return { true, false };
        }
    };

    template<int32 TDimensionSize, int32 TFirstValue=0>
    struct TShaderPermutationInt
    {
        using Type = int32;
        static constexpr bool IsMultiDimensional = false;
        static constexpr int32 PermutationCount = TDimensionSize;
        static constexpr Type MinValue = static_cast<Type>(TFirstValue);
        static constexpr Type MaxValue = static_cast<Type>(TFirstValue + TDimensionSize - 1);
        static int32 ToDimensionValueId(Type E)
        {
            int32 PermutationId = static_cast<int32>(E) - TFirstValue;
            check(PermutationId < PermutationCount && PermutationId >= 0);
            return PermutationId;
        }
        static int32 ToDefineValue(Type E)
        {
            return E;
        }
        static Type FromDimensionValueId(int32 PermutationId)
        {
            check(PermutationId < PermutationCount && PermutationId >= 0);
            return static_cast<Type>(PermutationId + TFirstValue);
        }
        static std::vector<int32> GetValueRange()
        {
            std::vector<int32> values;
            for (int i = MinValue; i <= MaxValue; i++)
                values.push_back(i);
            return values;
        }
    };

    
    /** Defines at compile time a permutation dimension made of specific int32. */
    template <int32... Ts>
    struct TShaderPermutationSparseInt
    {
        /** Setup the dimension's type in permutation domain as integer. */
        using Type = int32;

        /** Setup the dimension's number of permutation. */
        static constexpr int32 PermutationCount = 0;
        
        /** Setup the dimension as non multi-dimensional, so that the ModifyCompilationEnvironement's
         * define can conventily be set up in SHADER_PERMUTATION_SPARSE_INT.
         */
        static constexpr bool IsMultiDimensional = false;


        /** Converts dimension's integer value to dimension's value id, bu in this case fail because the dimension value was wrong. */
        static int32 ToDimensionValueId(Type E)
        {
            return int32(0);
        }

        /** Converts dimension's value id to dimension's integer value (exact reciprocal of ToDimensionValueId). */
        static Type FromDimensionValueId(int32 PermutationId)
        {
            return Type(0);
        }
    };

    template <int32 TUniqueValue, int32... Ts>
    struct TShaderPermutationSparseInt<TUniqueValue, Ts...>
    {
        /** Setup the dimension's type in permutation domain as integer. */
        using Type = int32;

        /** Setup the dimension's number of permutation. */
        static constexpr int32 PermutationCount = TShaderPermutationSparseInt<Ts...>::PermutationCount + 1;
        
        /** Setup the dimension as non multi-dimensional, so that the ModifyCompilationEnvironement's
         * define can conventily be set up in SHADER_PERMUTATION_SPARSE_INT.
         */
        static constexpr bool IsMultiDimensional = false;


        /** Converts dimension's integer value to dimension's value id. */
        static int32 ToDimensionValueId(Type E)
        {
            if (E == TUniqueValue)
            {
                return PermutationCount - 1;
            }
            return TShaderPermutationSparseInt<Ts...>::ToDimensionValueId(E);
        }

        /** Pass down a int32 to FShaderCompilerEnvironment::SetDefine(). */
        static int32 ToDefineValue(Type E)
        {
            return int32(E);
        }

        /** Converts dimension's value id to dimension's integer value (exact reciprocal of ToDimensionValueId). */
        static Type FromDimensionValueId(int32 PermutationId)
        {
            if (PermutationId == PermutationCount - 1)
            {
                return TUniqueValue;
            }
            return TShaderPermutationSparseInt<Ts...>::FromDimensionValueId(PermutationId);
        }
    };

    #define SHADER_PERMUTATION_BOOL(InDefineName) \
        public FShaderPermutationBool { \
        public: \
            static constexpr const char* DefineName = InDefineName; \
        }
    #define SHADER_PERMUTATION_INT(InDefineName, Count) \
        public TShaderPermutationInt<Count> { \
        public: \
            static constexpr const char* DefineName = InDefineName; \
        }
    #define SHADER_PERMUTATION_RANGE_INT(InDefineName, Start, Count) \
        public TShaderPermutationInt<Count, Start> { \
        public: \
            static constexpr const char* DefineName = InDefineName; \
        }
    #define SHADER_PERMUTATION_SPARSE_INT(InDefineName, ...) \
        public TShaderPermutationSparseInt<__VA_ARGS__> { \
        public: \
            static constexpr const char* DefineName = InDefineName; \
        }

    template<typename... Ts>
    struct TShaderPermutationDomain
    {
        /** Setup the dimension's type in permutation domain as itself so that a permutation domain can be
        * used as a dimension of another domain.
        */
        using Type = TShaderPermutationDomain<Ts...>;

        /** Define a domain as a multidimensional dimension so that ModifyCompilationEnvironment() is getting used. */
        static constexpr bool IsMultiDimensional = true;

        /** Total number of permutation within the domain is one if no dimension at all. */
        static constexpr int32 PermutationCount = 1;

        TShaderPermutationDomain<Ts...>() {}
        explicit TShaderPermutationDomain<Ts...>(int32 PermutationId)
        {
            check(PermutationId == 0);
        }

        template<class DimensionToSet>
        void Set(typename DimensionToSet::Type Value)
        {
            static_assert(sizeof(typename DimensionToSet::Type) == 0, "Unknown shader permutation dimension.");
        }

        template<class DimensionToGet>
        const typename DimensionToGet::Type Get() const
        {
            static_assert(sizeof(typename DimensionToGet::Type) == 0, "Unknown shader permutation dimension.");
            return DimensionToGet::Type();
        }

        static int32 ToDimensionValueId(const Type& PermutationVector)
        {
            return 0;
        }

        int32 ToDimensionValueId() const
        {
            return ToDimensionValueId(*this);
        }

        static Type FromDimensionValueId(const int32 PermutationId)
        {
            return Type(PermutationId);
        }

        void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment) const
        {

        }

        // static std::vector<int32> GetAllPermutationIds()
        // {
        //     return { ToDimensionValueId() };
        // }

        /** Test if equal. */
        bool operator==(const Type& Other) const
        {
            return true;
        }

        // template<class TPermutationVector>
        // static void GetAllPermutationIdsInternal(TPermutationVector &PermutationVector, std::vector<int32> &res)
        // {
        //     res.push_back(PermutationVector.ToDimensionValueId());
        // }

    };

    template<typename TDimension, typename... Ts>
    struct TShaderPermutationDomain<TDimension, Ts...>
    {
        /** Setup the dimension's type in permutation domain as itself so that a permutation domain can be
        * used as a dimension of another domain.
        */
        using Type = TShaderPermutationDomain<TDimension, Ts...>;

        /** Define a domain as a multidimensional dimension so that ModifyCompilationEnvironment() is used. */
        static constexpr bool IsMultiDimensional = true;

        /** Parent type in the variadic template to reduce code. */
        using Super = TShaderPermutationDomain<Ts...>;

        /** Total number of permutation within the domain. */
        static constexpr int32 PermutationCount = Super::PermutationCount * TDimension::PermutationCount;


        /** Constructors. */
        TShaderPermutationDomain<TDimension, Ts...>()
            : DimensionValue(TDimension::FromDimensionValueId(0))
        {
        }

        explicit TShaderPermutationDomain<TDimension, Ts...>(int32 PermutationId)
            : DimensionValue(TDimension::FromDimensionValueId(PermutationId % TDimension::PermutationCount))
            , Tail(PermutationId / TDimension::PermutationCount)
        {
            check(PermutationId >= 0 && PermutationId < PermutationCount);
        }

        template<class DimensionToSet>
        void Set(typename DimensionToSet::Type Value)
        {
            if constexpr (std::is_same<DimensionToSet, TDimension>::value)
            {
                DimensionValue = Value;
            }
            else 
            {
                Tail.Set<DimensionToSet>(Value);
            }
        }

        template<class DimensionToGet>
        const typename DimensionToGet::Type Get() const
        {
            if constexpr (std::is_same<DimensionToGet, TDimension>::value)
            {
                return DimensionValue;
            }
            else 
            {
                return Tail.Get<DimensionToGet>();
            }
        }

        /** Converts domain permutation vector to domain's value id. */
        static int32 ToDimensionValueId(const Type& PermutationVector)
        {
            return PermutationVector.ToDimensionValueId();
        }

        int32 ToDimensionValueId() const
        {
            return TDimension::ToDimensionValueId(DimensionValue) + TDimension::PermutationCount * Tail.ToDimensionValueId();
        }


        /** Returns the permutation domain from the unique ID. */
        static Type FromDimensionValueId(const int32 PermutationId)
        {
            return Type(PermutationId);
        }

        // static std::vector<int32> GetAllPermutationIds()
        // {
        //     std::vector<int32> res;
        //     TShaderPermutationDomain<TDimension, Ts...> PermutationVector;
        //     GetAllPermutationIdsInternal(PermutationVector, res);
        //     return res;
        // }

        void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment) const
        {

            if constexpr (TDimension::IsMultiDimensional)
            {                
                // This is designed for nested permutation domain
                // but it's not working now
                // so DON'T USE nested permutation domain
		        DimensionValue.ModifyCompilationEnvironment(OutEnvironment);
            }
            else 
            {
                OutEnvironment.SetDefine(TDimension::DefineName, TDimension::ToDefineValue(DimensionValue));
            }
            Tail.ModifyCompilationEnvironment(OutEnvironment);
        }

        /** Test if equal. */
        bool operator==(const Type& Other) const
        {
            return DimensionValue == Other.DimensionValue && Tail == Other.Tail;
        }

        /** Test if not equal. */
        bool operator!=(const Type& Other) const
        {
            return !(*this == Other);
        }

        // template<class TPermutationVector>
        // static void GetAllPermutationIdsInternal(TPermutationVector &PermutationVector, std::vector<int32> &res)
        // {
        //     if constexpr (TDimension::IsMultiDimensional)
        //     {
        //         // This is designed for nested permutation domain
        //         // but it's not working now
        //         // so DON'T USE nested permutation domain
        //         TDimension::Type::GetAllPermutationIdsInternal(PermutationVector, res);
        //         Super::GetAllPermutationIdsInternal(PermutationVector, res);
        //     }
        //     else
        //     {
        //         for (auto Value : TDimension::GetValueRange())
        //         {
        //             PermutationVector.Set<TDimension>(Value);
        //             Super::GetAllPermutationIdsInternal(PermutationVector, res);
        //         }
        //     }
        // }

    private:
        typename TDimension::Type DimensionValue;
        Super Tail;
    };

    using FShaderPermutationNone = TShaderPermutationDomain<>;
}