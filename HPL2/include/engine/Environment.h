/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

// NOTE: none of these classes are thread safe

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>

#include <math/Uuid.h>

#include <folly/container/F14Map.h>
#include "Common_3/Utilities/Interfaces/ILog.h"
#include "FixPreprocessor.h"

namespace hpl {

    class EnvironmentStore;

    struct EnvironmentResult {
    public:
        enum ResultStatus {
            Found,
            Created,
            NotFound,
            OutOfMemory,
        };
        ResultStatus m_status;
        void* m_variable;
    };

    template<class T>
    class EnvironmentVariableHolder final {
    public:
        using Type = T;

        EnvironmentVariableHolder(uint32_t id, EnvironmentStore* environment)
            : m_id(id)
            , m_environmentOwner(environment) {
            m_refCount = 0;
            m_isConstructed = false;
        }
        ~EnvironmentVariableHolder() {
        }

        void AddRef();
        void DecrementRef();

        template<class... TArgs>
        void Construct(TArgs&&... args) {
            if (!m_isConstructed) {
                m_isConstructed = true;
#if __cpp_lib_launder
                std::construct_at(std::launder(reinterpret_cast<T*>(&m_value)), std::forward<TArgs>(args)...);
#else
                std::construct_at(reinterpret_cast<T*>(&m_value), AZStd::forward<TArgs>(args)...);
#endif
            }
        }

        T* Instance() {
            return reinterpret_cast<T*>(&m_value);
        }

        bool IsConstructed() const {
            return m_isConstructed;
        }

    private:
        uint32_t m_id = 0;
        uint16_t m_refCount = 0;
        EnvironmentStore* m_environmentOwner;
        bool m_isConstructed = false;
        std::aligned_storage_t<sizeof(T), alignof(T)> m_value;
    };

    template<class T>
    class EnvironmentVariableContainer final {
    public:
        using HolderType = EnvironmentVariableHolder<T>;

        EnvironmentVariableContainer()
            : m_holder(nullptr) {
        }
        EnvironmentVariableContainer(HolderType* holder)
            : m_holder(holder) {
            if (m_holder) {
                m_holder->AddRef();
            }
        }

        ~EnvironmentVariableContainer() {
            if (m_holder) {
                m_holder->DecrementRef();
            }
        }

        T& operator*() {
            ASSERT(m_holder && "You can't dereference a null pointer");
            return *reinterpret_cast<T*>(&m_holder->Instance());
        }

        T* operator->() {
            ASSERT(m_holder && "You can't dereference a null pointer");
            return reinterpret_cast<T*>(&m_holder->Instance());
        }

        T& Get() {
            ASSERT(m_holder && "You can't dereference a null pointer");
            return *GetPtr();
        }

        T* GetPtr() {
            return m_holder ? reinterpret_cast<T*>(m_holder->Instance()) : nullptr;
        }

        void Reset() {
            if (m_holder) {
                m_holder->DecrementRef();
            }
            m_holder = nullptr;
        }

        void Swap(EnvironmentVariableContainer& other) {
            HolderType* temp = m_holder;
            m_holder = other.m_holder;
            other.m_holder = temp;
        }

        bool IsConstructed() {
            return m_holder && m_holder->IsConstructed();
        }

    private:
        HolderType* m_holder;
    };

    template<class T>
    inline bool operator==(T* lhs, const EnvironmentVariableContainer<T>& rhs) {
        return lhs == rhs.GetPtr();
    }

    template<class T>
    inline bool operator==(EnvironmentVariableContainer<T> const& lhs, T* rhs) {
        return lhs.GetPtr() == rhs;
    }

    template<class T>
    inline bool operator==(EnvironmentVariableContainer<T> const& lhs, EnvironmentVariableContainer<T> const& rhs) {
        return lhs.GetPtr() == rhs.GetPtr();
    }

    template<class T>
    inline bool operator!=(EnvironmentVariableContainer<T> const& lhs, EnvironmentVariableContainer<T> const& rhs) {
        return !(lhs == rhs);
    }

    template<class T>
    inline bool operator!=(T* lhs, EnvironmentVariableContainer<T> const& rhs) {
        return !(lhs == rhs);
    }

    template<class T>
    inline bool operator!=(EnvironmentVariableContainer<T> const& lhs, T* rhs) {
        return !(lhs == rhs);
    }

    class EnvironmentStore final {
    public:
        EnvironmentStore();
        ~EnvironmentStore();

        //EnvironmentResult AddVariable(uint32_t id, void* variable);
        EnvironmentResult AddOrAllocateVariable(uint32_t id, size_t size);
        EnvironmentResult RemoveVariable(uint32_t id);
        EnvironmentResult RemoveAndDeallocateVariable(uint32_t id);
        EnvironmentResult GetVariable(uint32_t id);

    private:
        folly::F14ValueMap<uint32_t, void*> m_variableMap;
    };

    namespace Environment {
        EnvironmentStore& GetStore();

        template<class T, class... TArgs>
        EnvironmentVariableContainer<T> CreateVariable(const std::string_view& name, TArgs&&... args);

        template<class T, class... TArgs>
        EnvironmentVariableContainer<T> CreateVariable(uint32_t guid, TArgs&&... args);

        template<class T>
        EnvironmentVariableContainer<T> FindVariable(uint32_t guid);
    } // namespace Environment

    template<class T, class... TArgs>
    EnvironmentVariableContainer<T> Environment::CreateVariable(uint32_t guid, TArgs&&... args) {
        ASSERT(guid != 0 && "You can't create a variable with a null guid");
        using HolderType = typename EnvironmentVariableContainer<T>::HolderType;

        auto& store = GetStore();

        HolderType* variable = nullptr;
        auto result = store.AddOrAllocateVariable(guid, sizeof(HolderType));
        if (result.m_status == EnvironmentResult::Created) {
            variable = new (result.m_variable) HolderType(guid, &Environment::GetStore());
        } else if (result.m_status == EnvironmentResult::Found) {
            variable = reinterpret_cast<HolderType*>(result.m_variable);
        }
        if (variable) {
            variable->Construct(std::forward<TArgs>(args)...);
        }
        return EnvironmentVariableContainer<T>(variable);
    }

    template<class T, class... TArgs>
    EnvironmentVariableContainer<T> Environment::CreateVariable(const std::string_view& name, TArgs&&... args) {
        return CreateVariable<T>(hpl::detail::From(name), std::forward<TArgs>(args)...);
    }

    template<class T>
    EnvironmentVariableContainer<T> Environment::FindVariable(uint32_t guid) {
        using HolderType = typename EnvironmentVariableContainer<T>::HolderType;
        auto& store = GetStore();
        auto entry = store.GetVariable(guid);
        if (entry.m_status == EnvironmentResult::Found) {
            auto result = reinterpret_cast<HolderType*>(entry.m_variable);
            return EnvironmentVariableContainer<T>(result);
        }
        return EnvironmentVariableContainer<T>(nullptr);
    }

    template<class T>
    void EnvironmentVariableHolder<T>::AddRef() {
        ++m_refCount;
    }

    template<class T>
    void EnvironmentVariableHolder<T>::DecrementRef() {
        m_refCount--;
        if (m_refCount == 0) {
            if (m_isConstructed) {
                m_isConstructed = false;
#if __cpp_lib_launder
                std::destroy_at(std::launder(reinterpret_cast<T*>(&m_value)));
#else
                std::destroy_at(reinterpret_cast<T*>(&m_value));
#endif
            }
            if (m_environmentOwner) {
                this->~EnvironmentVariableHolder<T>();
                m_environmentOwner->RemoveAndDeallocateVariable(m_id);
            }
            m_environmentOwner = nullptr;
        }
    }

} // namespace hpl
