#pragma once 

#include <math/Uuid.h>
#include <string_view>

#define HPL_RTTI_CLASS(name, id) \
    public: \
        static constexpr const char* RTTI_Name = "#name"; \
        static constexpr const hpl::Uuid RTTI_ClassID = hpl::detail::From(#id); \
        virtual bool RTTI_IsSubtype(const hpl::Uuid& classID) { return classID == RTTI_ClassID; } \
        virtual const char* RTTI_GetName() { return RTTI_Name; } \
        virtual const hpl::Uuid RTTI_GetClassID() { return RTTI_ClassID; } \
    private: \

#define HPL_RTTI_IMPL_CLASS(base, name, id) \
    public: \
        static constexpr const char* RTTI_Name = "#name"; \
        static constexpr const hpl::Uuid RTTI_ClassID = hpl::detail::From(#id); \
        virtual bool RTTI_IsSubtype(const hpl::Uuid& classID) override { return classID == RTTI_GetClassID() || base::RTTI_IsSubtype(classID); } \
        virtual const char* RTTI_GetName() override { return RTTI_Name; } \
        virtual const hpl::Uuid RTTI_GetClassID() override { return RTTI_ClassID; } \
    private: \


namespace hpl {
    template<typename T>
    class TypeInfo {
    public:
        static hpl::Uuid GetClassID() {
            return T::RTTI_ClassID;
        }

        template<typename U>
        static bool IsType(U& impl) {
            return impl.RTTI_GetClassID() == T::RTTI_ClassID;
        }

        template<typename U>
        static bool IsSubtype(U& impl) {
            return impl.RTTI_IsSubtype(T::RTTI_ClassID);
        }

        static std::string_view GetName() {
            return T::RTTI_Name;
        }
    };
}
