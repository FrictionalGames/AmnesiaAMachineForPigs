/**
 * Copyright 2023 Michael Pollind
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

#include <engine/Event.h>
#include <engine/RTTI.h>
#include <math/MathTypes.h>
#include <system/HandleWrapper.h>

#include <SDL2/SDL_events.h>

#include "Common_3/OS/Interfaces/IOperatingSystem.h"
#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

namespace hpl::window {

    enum class WindowEventType : uint16_t {
        ResizeWindowEvent,
        MoveWindowEvent,
        QuitEvent,
        // Mouse can move to a separate interface
        MouseMoveEvent,
        // keyboard can move to a separate interface

    };
    enum class WindowStyle : uint32_t {
        WindowStyleNone = 0,
        WindowStyleTitleBar = 0x01,
        WindowStyleResizable = 0x02,
        WindowStyleBorderless = 0x04,
        WindowStyleFullscreen = 0x08,
        WindowStyleFullscreenDesktop = 0x10,

        WindowStyleClosable = 0x20,
        WindowStyleMinimizable = 0x40,
        WindowStyleMaximizable = 0x80,
    };

    enum class WindowStatus : uint32_t {
        WindowStatusNone = 0,
        WindowStatusInputFocus = 0x01,
        WindowStatusInputMouseFocus = 0x02,
        WindowStatusWindowMinimized = 0x04,
        WindowStatusWindowMaximized = 0x08,
        WindowStatusVisible = 0x10,
    };

    struct InternalEvent {
        union {
            SDL_Event m_sdlEvent; // SDL_Event when compiling for SDL2
        };
    };

    struct WindowEventPayload {
        WindowEventType m_type;
        union {
            struct {
                uint32_t m_width;
                uint32_t m_height;
            } m_resizeWindow;
            struct {
                int32_t m_x;
                int32_t m_y;
            } m_moveWindow;
            struct {
                int32_t m_x;
                int32_t m_y;
            } m_mouseEvent;
        } payload;
    };
    using WindowEvent = hpl::Event<WindowEventPayload&>;

    enum class WindowType : uint8_t { Window, Fullscreen, Borderless };
    namespace internal {

        using WindowInternalEvent = hpl::Event<InternalEvent&>;
        class NativeWindowHandler final {
            HPL_HANDLER_IMPL(NativeWindowHandler)
        };

        // this is internal to the windowing system and should not be used by the user ...
        void ConnectInternalEventHandler(NativeWindowHandler& handler, WindowInternalEvent::Handler& eventHandle);
        void ConnectionWindowEventHandler(NativeWindowHandler& handler, WindowEvent::Handler& eventHandle);

        WindowInternalEvent& NativeInternalEvent(NativeWindowHandler& handler);
        WindowEvent& NativeWindowEvent(NativeWindowHandler& handler);

        NativeWindowHandler Initialize(const WindowStyle& style);
        void SetWindowTitle(NativeWindowHandler& handler, const std::string_view title);
        void SetWindowSize(NativeWindowHandler& handler, const cVector2l& size);
        void SetWindowBrightness(NativeWindowHandler& handler, float brightness);

        WindowHandle ForgeWindowHandle(NativeWindowHandler& handler);
        cVector2l GetWindowSize(NativeWindowHandler& handler);
        WindowStatus GetWindowStatus(NativeWindowHandler& handler);

        void WindowGrabCursor(NativeWindowHandler& handler);
        void WindowReleaseCursor(NativeWindowHandler& handler);
        void ShowHardwareCursor(NativeWindowHandler& handler);
        void HideHardwareCursor(NativeWindowHandler& handler);
        void ConstrainCursor(NativeWindowHandler& handler);
        void UnconstrainCursor(NativeWindowHandler& handler);

        void Process(NativeWindowHandler& handler);
    } // namespace internal

    // wrapper over an opaque pointer to a windowing system
    // this is the only way to interact with the windowing system
    class NativeWindowWrapper final {
        HPL_RTTI_CLASS(NativeWindow, "{d17ea5c7-30f1-4d5d-b38e-1a7e88e137fc}")
    public:
        ~NativeWindowWrapper() {
        }
        NativeWindowWrapper() = default;
        NativeWindowWrapper(internal::NativeWindowHandler&& handle)
            : m_impl(std::move(handle)) {
        }
        NativeWindowWrapper(NativeWindowWrapper&& other)
            : m_impl(std::move(other.m_impl)) {
        }
        NativeWindowWrapper(const NativeWindowWrapper& other) = delete;

        NativeWindowWrapper& operator=(NativeWindowWrapper& other) = delete;
        void operator=(NativeWindowWrapper&& other) {
            m_impl = std::move(other.m_impl);
        }

        inline void SetWindowSize(cVector2l size) {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::SetWindowSize(m_impl, size);
        }

        inline void SetWindowTitle(const std::string_view title) {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::SetWindowTitle(m_impl, title);
        }

        inline void SetWindowBrightness(float brightness) {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::SetWindowBrightness(m_impl, brightness);
        }

        inline cVector2l GetWindowSize() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            return internal::GetWindowSize(m_impl);
        }

        inline void ConnectWindowEventHandler(WindowEvent::Handler& handler) {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::ConnectionWindowEventHandler(m_impl, handler);
        }

        inline void ConnectInternalEventHandler(internal::WindowInternalEvent::Handler& handler) {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::ConnectInternalEventHandler(m_impl, handler);
        }

        inline internal::WindowInternalEvent& NativeInternalEvent() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            return internal::NativeInternalEvent(m_impl);
        }

        inline WindowHandle ForgeWindowHandle() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            return internal::ForgeWindowHandle(m_impl);
        }

        inline WindowEvent& NativeWindowEvent() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            return internal::NativeWindowEvent(m_impl);
        }


        inline WindowStatus GetWindowStatus() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            return internal::GetWindowStatus(m_impl);
        }

        inline void Process() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::Process(m_impl);
        }

        inline void GrabCursor() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::WindowGrabCursor(m_impl);
        }
        inline void ReleaseCursor() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::WindowReleaseCursor(m_impl);
        }
        inline void ShowHardwareCursor() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::ShowHardwareCursor(m_impl);
        }
        inline void HideHardwareCursor() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::HideHardwareCursor(m_impl);
        }
        inline void ConstrainCursor() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::ConstrainCursor(m_impl);
        }
        inline void UnconstrainCursor() {
            ASSERT(m_impl && "NativeWindowHandle is null");
            internal::UnconstrainCursor(m_impl);
        }

        inline WindowType GetWindowType() {
            return WindowType::Window;
        }

    private:
        internal::NativeWindowHandler m_impl = internal::NativeWindowHandler();
    };

    inline hpl::window::WindowStyle operator&(hpl::window::WindowStyle lhs, hpl::window::WindowStyle rhs) {
        return static_cast<hpl::window::WindowStyle>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }
    inline hpl::window::WindowStyle operator|(hpl::window::WindowStyle lhs, hpl::window::WindowStyle rhs) {
        return static_cast<hpl::window::WindowStyle>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }
    inline bool any(hpl::window::WindowStyle lhs) {
        return static_cast<uint32_t>(lhs) != 0;
    }

    inline hpl::window::WindowStatus operator&(hpl::window::WindowStatus lhs, hpl::window::WindowStatus rhs) {
        return static_cast<hpl::window::WindowStatus>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }
    inline hpl::window::WindowStatus operator|(hpl::window::WindowStatus lhs, hpl::window::WindowStatus rhs) {
        return static_cast<hpl::window::WindowStatus>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }
    inline bool any(hpl::window::WindowStatus lhs) {
        return static_cast<uint32_t>(lhs) != 0;
    }

} // namespace hpl::window
