#pragma once
#include <queue>
#include <string>
#include <UDRefl/UDRefl.hpp>


template<typename T>
struct TClassRegistry { };

class NClass
{
public:

    template<typename T>
    friend class TClassRegistry;

    NClass() = default;

    bool IsChildOf(const NClass *BaseClass) const
    {
        if (Type == BaseClass->Type)
            return true;
        std::queue<Ubpa::Type> q;
        q.push(Type);
        while (!q.empty())
        {
            Ubpa::Type temp_class = q.front(); q.pop();
            auto temp_info = Ubpa::UDRefl::Mngr.GetTypeInfo(temp_class);
            for (auto& [parent_class, base_info] : temp_info->baseinfos)
            {
                if (parent_class == BaseClass->Type)
                    return true;
                q.push(parent_class);
            }
        }
        return false;
    }

    inline bool operator==(const NClass &Other) const
    {
        return Type == Other.Type;
    }

    inline bool operator<(const NClass &Other) const
    {
        return Type < Other.Type;
    }

    Ubpa::Type GetType() const
    {
        return Type;
    }

    const Ubpa::UDRefl::TypeInfo* GetTypeInfo() const
    {
        return TypeInfo;
    }

private:

    const Ubpa::UDRefl::TypeInfo *TypeInfo;

    Ubpa::Type Type;

};
