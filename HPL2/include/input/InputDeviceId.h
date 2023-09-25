#pragma once

#include "math/Crc32.h"
#include <array>
#include <cstdint>

#include <string>
#include <string_view>

namespace hpl::input {
    class InputDeviceID {
    public:
        static constexpr size_t MaxNameLength = 95;

        constexpr InputDeviceID(const std::string_view name, uint8_t index = 0):
            m_index(index) {
            math::Crc32 value;
            value.Update(name);
            value.Update(index);
            m_id = value;
        }

        const math::Crc32& GetID() const {
            return m_id;
        }

        // const std::string_view GetName() const {
        //     return m_name;
        // }

        uint8_t GetIndex() const {
            return m_index;
        }

        constexpr bool operator==(const InputDeviceID& other) const {
            return m_id == other.m_id;
        }
    private:
        uint8_t m_index;
        // std::string m_name;
        math::Crc32 m_id;
    };

}
