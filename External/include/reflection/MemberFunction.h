#pragma once
#include <functional>
#include <any>
#include <string>
#include <stdexcept>
#include <array>
#include <optional>

namespace reflection {

    class ArgWrap
    {
    public:

        template<typename T>
        ArgWrap(T&& Value)
        {
            bIsConst = std::is_const_v<T>;
            bIsRefType = std::is_reference_v<T>;
            if (bIsRefType)
            {
                this->Storage = &Value;
            }
            else 
            {
                this->Storage = Value;
            }
        }

        template<typename T>
        T Cast()
        {
            using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
            constexpr bool bCastIsRefType = std::is_reference_v<T>;
            if constexpr (bCastIsRefType == false)
            {
                if (bIsRefType)
                {
                    // ref to value type
                    return *std::any_cast<const RawT*>(Storage);
                }
                else 
                {
                    // value type to value type
                    return std::any_cast<RawT>(Storage);
                }
            }
            else 
            {
                if (bIsRefType)
                {
                    constexpr bool bCastIsConst = std::is_const_v<T>;
                    if constexpr (bCastIsConst)
                    {
                        if (bIsConst)
                            // const ref to const ref
                            return *std::any_cast<const RawT*>(Storage);
                        else
                            // non-const ref to const ref
                            return *std::any_cast<RawT*>(Storage);
                    }
                    else 
                    {
                        if (bIsConst)
                            // const ref to non-const ref, fail
                            throw std::runtime_error("Cannot cast const ref to non-const ref");
                        else
                            // non-const ref to non-const ref
                            return *std::any_cast<RawT*>(Storage);
                    }

                }
                else 
                {
                    // value type to value type
                    return *std::any_cast<RawT>(&Storage);
                }
            }
        }

    private:

        bool bIsConst;
        bool bIsRefType;

        std::any Storage;

    };

    class MemberFunction
    {
    public:

        friend class RawTypeDescriptorBuilder;

        template<typename TClass, typename ...TArgs>
        explicit MemberFunction(void (TClass::*Function)(TArgs...))
        {
            ArgsNum = sizeof...(TArgs);
            function = [Function](void* pArgsArray) -> std::any {
                auto pArgs = static_cast<std::array<ArgWrap, sizeof...(TArgs)+1>*>(pArgsArray);
                auto ArgsTuple = AsTuple<TClass&, TArgs...>(*pArgs);
                std::apply(Function, ArgsTuple);
                return std::any{};
            };
        }

        template<typename TClass, typename TReturn, typename ...TArgs>
        explicit MemberFunction(TReturn (TClass::*Function)(TArgs...))
        {
            ArgsNum = sizeof...(TArgs);
            function = [Function](void* pArgsArray) -> std::any {
                auto pArgs = static_cast<std::array<ArgWrap, sizeof...(TArgs)+1>*>(pArgsArray);
                auto ArgsTuple = AsTuple<TClass&, TArgs...>(*pArgs);
                return std::apply(Function, ArgsTuple);
            };
        }

        template<typename TClass, typename ...TArgs>
        explicit MemberFunction(void (TClass::*Function)(TArgs...) const)
        {
            ArgsNum = sizeof...(TArgs);
            function = [Function](void* pArgsArray) -> std::any {
                auto pArgs = static_cast<std::array<ArgWrap, sizeof...(TArgs)+1>*>(pArgsArray);
                auto ArgsTuple = AsTuple<TClass&, TArgs...>(*pArgs);
                std::apply(Function, ArgsTuple);
                return std::any{};
            };
            bIsConst = true;
        }

        template<typename TClass, typename TReturn, typename ...TArgs>
        explicit MemberFunction(TReturn (TClass::*Function)(TArgs...) const)
        {
            ArgsNum = sizeof...(TArgs);
            function = [Function](void* pArgsArray) -> std::any {
                auto pArgs = static_cast<std::array<ArgWrap, sizeof...(TArgs)+1>*>(pArgsArray);
                auto ArgsTuple = AsTuple<TClass&, TArgs...>(*pArgs);
                return std::apply(Function, ArgsTuple);
            };
            bIsConst = true;
        }

        template<typename TClass, typename ...TArgs>
        void Invoke(TClass&& Object, TArgs&&... Args) const
        {
            if (ArgsNum != sizeof...(TArgs))
                throw std::runtime_error("Mismatching number of arguments");
            
            std::array<ArgWrap, sizeof...(TArgs)+1> ArgsArray = {
                ArgWrap(Object),
                ArgWrap(std::forward<TArgs>(Args))...
            };
            function(&ArgsArray);
        }

        template<typename TClass, typename TReturn, typename ...TArgs>
        TReturn Invoke(TClass&& Object, TArgs&&... Args) const
        {
            if (ArgsNum != sizeof...(TArgs))
                throw std::runtime_error("Mismatching number of arguments");
            
            std::array<ArgWrap, sizeof...(TArgs)+1> ArgsArray = {
                ArgWrap(Object),
                ArgWrap(std::forward<TArgs>(Args))...
            };
            return function(&ArgsArray);
        }

        const std::string& GetFunctionName() const
        {
            return FunctionName;
        }

    private:

        std::function<std::any(void*)> function;

        bool bIsConst;

        int ArgsNum;

        std::string FunctionName;

        template<typename... Args, size_t N, size_t... Is>
        static std::tuple<Args...> AsTuple(std::array<ArgWrap, N>& array, std::index_sequence<Is...>) 
        {
            return std::forward_as_tuple(array[Is].template Cast<Args>()...);
        }

        template<typename... Args, size_t N, typename = std::enable_if_t<N == sizeof...(Args)>>
        static std::tuple<Args...> AsTuple(std::array<ArgWrap, N>& array) 
        {
            return AsTuple<Args...>(array, std::make_index_sequence<N>());
        }

    };

    class Constructor
    {
    public:

        template<typename TClass>
        Constructor(TClass*)
        {
            function = []() {
                return new TClass;
            };
        }

        void* Invoke() const
        {
            return function();
        }

        std::function<void*(void)> function;

    };

}