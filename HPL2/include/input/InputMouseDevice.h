#pragma once

#include <input/InputManager.h>
#include <input/InputDevice.h>
#include <system/HandleWrapper.h>
#include <cstdint>
#include <engine/RTTI.h>
#include <math/MathTypes.h>
#include <math/Uuid.h>
#include <memory>

#include <string_view>
#include <windowing/NativeWindow.h>
#include <input/BaseInputDevice.h>

namespace hpl::input {

    // enum class MouseButton : uint8_t { Left, Middle, Right, WheelUp, WheelDown, Button6, Button7, Button8, Button9, LastEnum };
    namespace detail {
        std::string_view MouseButtonToString(eMouseButton button);
        eMouseButton StringToMouseButton(const std::string_view& button);
    }

    namespace internal::mouse {
        class InternalInputMouseHandle final {
            HPL_HANDLER_IMPL(InternalInputMouseHandle)
        };

        InternalInputMouseHandle Initialize();
        cVector2l GetAbsPosition(InternalInputMouseHandle& handle);
        cVector2l GetRelPosition(InternalInputMouseHandle& handle);

        bool IsButtonPressed(InternalInputMouseHandle& handle, eMouseButton button);

        window::internal::WindowInternalEvent::Handler& GetWindowEventHandle(InternalInputMouseHandle& handle); // internal use only
    } // namespace internal

    // wrapper over the internal implementation
    // this class is copyable and movable
    class InputMouseDevice final : public BaseInputDevice {
        HPL_RTTI_IMPL_CLASS(BaseInputDevice, InputMouseDevice, "{ac1b28f3-7a0f-4442-96bb-99b64adb5be6}")
    public:
        InputMouseDevice() = default;
        InputMouseDevice(internal::mouse::InternalInputMouseHandle&& handle) :
            m_impl(std::move(handle)) {
        }
        InputMouseDevice(InputMouseDevice&& other)
            : m_impl(std::move(other.m_impl)) {
        }
        void operator=(InputMouseDevice&& other) {
            m_impl = std::move(other.m_impl);
        }

        // disallow copy
        void operator=(const InputMouseDevice& other) = delete;
        InputMouseDevice(const InputMouseDevice& other) = delete;
        ~InputMouseDevice() = default;

        /**
         * Check if a mouse button is down
         * \param eMouseButton the button to check
         * \return
         */
        bool ButtonIsDown(eMouseButton button) {
            return internal::mouse::IsButtonPressed(m_impl, button);
        }
        /**
         * Get the absolute pos of the mouse.
         * \return
         */
        cVector2l GetAbsPosition() {
            return internal::mouse::GetAbsPosition(m_impl);
        }
        /**
         * Get the relative movement.
         * \return
         */
        cVector2l GetRelPosition() {
            return internal::mouse::GetRelPosition(m_impl);
        }

    private:
        internal::mouse::InternalInputMouseHandle m_impl;
    };
} // namespace hpl::input