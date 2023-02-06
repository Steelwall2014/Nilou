#pragma once
#include <map>
#include <vector>

#include "Platform.h"

namespace nilou {

    class FDelegateHandle
    {
    public:
        enum EGenerateNewHandleType
        {
            GenerateNewHandle
        };

        /** Creates an initially unset handle */
        FDelegateHandle()
            : ID(0)
        {
        }

        /** Creates a handle pointing to a new instance */
        explicit FDelegateHandle(EGenerateNewHandleType)
            : ID(GenerateNewID())
        {
        }

        /** Returns true if this was ever bound to a delegate, but you need to check with the owning delegate to confirm it is still valid */
        bool IsValid() const
        {
            return ID != 0;
        }

        /** Clear handle to indicate it is no longer bound */
        void Reset()
        {
            ID = 0;
        }

    private:
        friend bool operator==(const FDelegateHandle& Lhs, const FDelegateHandle& Rhs)
        {
            return Lhs.ID == Rhs.ID;
        }

        friend bool operator!=(const FDelegateHandle& Lhs, const FDelegateHandle& Rhs)
        {
            return Lhs.ID != Rhs.ID;
        }

        /**
        * Generates a new ID for use the delegate handle.
        *
        * @return A unique ID for the delegate.
        */
        static uint64 GenerateNewID();

        uint64 ID;
    };

    template <typename... ParamTypes>
    class IDelegateInstance
    {
    public:
        virtual void Execute(ParamTypes...) = 0;
        virtual FDelegateHandle GetHandle() const = 0;
    };

    template <typename UserClass, typename... ParamTypes>
    class TDelegateInstance : public IDelegateInstance<ParamTypes...>
    {
    public:

        TDelegateInstance(UserClass *InUserObject, void (UserClass:: *InMethod)(ParamTypes...))
            : UserObject(InUserObject)
            , Method(InMethod)
            , Handle(FDelegateHandle::GenerateNewHandle)
        { }

        virtual void Execute(ParamTypes... params) override
        {
            (UserObject->*Method)(params...);
        }

        virtual FDelegateHandle GetHandle() const override { return Handle; }
    
    protected:
        UserClass *UserObject;
		void (UserClass:: *Method)(ParamTypes...);
        FDelegateHandle Handle;
    };

    template <typename... ParamTypes>
    class TDelegate
    {
    public:
        template <typename UserClass>
        FDelegateHandle Bind(UserClass *InUserObject, void (UserClass:: *InMethod)(ParamTypes...))
        {
            if (DelegateInstance)
                delete DelegateInstance;
            DelegateInstance = new TDelegateInstance<UserClass, ParamTypes...>(InUserObject, InMethod);
            if (DelegateInstance)
                return DelegateInstance->GetHandle();
            else 
                return FDelegateHandle();
        }

        void Execute(ParamTypes... InParams)
        {
            if (DelegateInstance)
                DelegateInstance->Execute(InParams...);
        }

    protected:
        IDelegateInstance<ParamTypes...> *DelegateInstance = nullptr;
    };

    template <typename... ParamTypes>
    class TMulticastDelegate
    {
    public:
        template <typename UserClass>
        FDelegateHandle Add(TDelegateInstance<UserClass, ParamTypes...> *DelegateInstance)
        {
            DelegateInstances.push_back(DelegateInstance);
            if (DelegateInstance)
                return DelegateInstance->GetHandle();
            else 
                return FDelegateHandle();
        }

        template <typename UserClass>
        FDelegateHandle Add(UserClass *InUserObject, void (UserClass:: *InMethod)(ParamTypes...))
        {
            auto DelegateInstance = new TDelegateInstance<UserClass, ParamTypes...>(InUserObject, InMethod);
            DelegateInstances.push_back(DelegateInstance);
            if (DelegateInstance)
                return DelegateInstance->GetHandle();
            else 
                return FDelegateHandle();
        }

        template <typename UserClass>
        bool Remove(FDelegateHandle Handle)
        {
            for (auto iter = DelegateInstances.end()-1; iter >= DelegateInstances.begin(); iter--)
            {
                if ((*iter)->GetHandle() == Handle)
                {
                    DelegateInstances.erase(iter);
                    return true;
                }
            }
            return false;
        }

        void Broadcast(ParamTypes... InParams)
        {
            for (IDelegateInstance<ParamTypes...> *DelegateInstance : DelegateInstances)
            {
                DelegateInstance->Execute(InParams...);
            }
        }

    protected:
        std::vector<IDelegateInstance<ParamTypes...> *> DelegateInstances;
    };

}