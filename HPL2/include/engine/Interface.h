
#pragma once

#include <cstdint>
#include <engine/Environment.h>

namespace hpl {

    template<class T>
    class Interface {
    public:
        static void Register(T* instance);
        static void UnRegister(T* instance);
        static T* Get();

        static uint32_t GetClassID() {
            return hpl::detail::ToGUID(T::RTTI_ClassID).m_data1;
        }

    private:
        inline static hpl::EnvironmentVariableContainer<T*>& GetInstance() {
            /**
             * Module-specific static environment variable. This will require a FindVariable<>() operation
             * when invoked for the first time in a new module. There is one of these per module, but they
             * all point to the same internal pointer.
             */
            static hpl::EnvironmentVariableContainer<T*> s_instance(nullptr);
            return s_instance;
        }
    };

    template<class T>
    void Interface<T>::Register(T* instance) {
        auto id = GetClassID();
        ASSERT(!hpl::Environment::FindVariable<T*>(id).IsConstructed() && "Interface already registered");

        auto inst = hpl::Environment::CreateVariable<T*>(id);
        GetInstance().Swap(inst);
        GetInstance().Get() = instance;
    }

    template<class T>
    void Interface<T>::UnRegister(T* instance) {
        ASSERT(GetInstance().IsConstructed() && "Unregistering wrong instance");
        ASSERT(GetInstance().Get() == instance && "Unregistering wrong instance");

        auto id = GetClassID();
        GetInstance().Reset();
    }

    template<class T>
    T* Interface<T>::Get() {
        auto& instance = GetInstance();

        auto id = GetClassID();
        return instance.IsConstructed() ? instance.Get() : nullptr;
    }
} // namespace hpl