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


#include <input/InputMouseDevice.h>

#include <engine/IUpdateEventLoop.h>

#include "engine/Interface.h"
#include "input/InputTypes.h"
#include "math/MathTypes.h"
#include "windowing/NativeWindow.h"
#include <memory>

#include <input/InputKeyboardDevice.h>
#include <input/InputMouseDevice.h>

#include <folly/small_vector.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

namespace hpl::input::internal {

    namespace mouse {
        struct InternalInputMouseImpl {
            window::internal::WindowInternalEvent::Handler m_windowEventHandle;
            cVector2l m_mousePosition;
            cVector2l m_mouseRelPosition;
            std::array<bool, static_cast<size_t>(eMouseButton::eMouseButton_LastEnum)> m_mouseButton;
        };


        bool IsButtonPressed(InternalInputMouseHandle& handle, eMouseButton button) {
            auto* impl = static_cast<InternalInputMouseImpl*>(handle.Get());
            ASSERT(button < impl->m_mouseButton.size() && "Invalid mouse button");
            return impl->m_mouseButton[static_cast<size_t>(button)];
        }

        InternalInputMouseHandle Initialize() {
            auto ptr = InternalInputMouseHandle::Ptr(new InternalInputMouseImpl(), [](void* ptr) {
                auto impl = static_cast<InternalInputMouseImpl*>(ptr);
                impl->m_windowEventHandle.Disconnect();
                delete impl;
            });
            InternalInputMouseHandle handle = InternalInputMouseHandle(std::move(ptr));

            auto* impl = static_cast<InternalInputMouseImpl*>(handle.Get());
            impl->m_windowEventHandle = window::internal::WindowInternalEvent::Handler([impl](hpl::window::InternalEvent& event) {
                auto& sdlEvent = event.m_sdlEvent;

                switch (sdlEvent.type) {
                case SDL_EventType::SDL_MOUSEMOTION:
                    impl->m_mousePosition = cVector2l(sdlEvent.motion.x, sdlEvent.motion.y);
                    impl->m_mouseRelPosition = cVector2l(sdlEvent.motion.xrel, sdlEvent.motion.yrel);
                    break;
                case SDL_EventType::SDL_MOUSEBUTTONDOWN:
                case SDL_EventType::SDL_MOUSEBUTTONUP:
                    {
                        const bool isPressed = sdlEvent.button.state == SDL_PRESSED;
                        switch (sdlEvent.button.button) {
                        case SDL_BUTTON_LEFT:
                            impl->m_mouseButton[static_cast<size_t>(eMouseButton_Left)] = isPressed;
                            break;
                        case SDL_BUTTON_MIDDLE:
                            impl->m_mouseButton[static_cast<size_t>(eMouseButton_Middle)] = isPressed;
                            break;
                        case SDL_BUTTON_RIGHT:
                            impl->m_mouseButton[static_cast<size_t>(eMouseButton_Right)] = isPressed;
                            break;
                        case SDL_BUTTON_X1:
                            impl->m_mouseButton[static_cast<size_t>(eMouseButton_Button6)] = isPressed;
                            break;
                        case SDL_BUTTON_X2:
                            impl->m_mouseButton[static_cast<size_t>(eMouseButton_Button7)] = isPressed;
                            break;
                        }
                        break;
                    }
                case SDL_EventType::SDL_MOUSEWHEEL:
                    if (sdlEvent.wheel.y > 0) {
                        impl->m_mouseButton[static_cast<size_t>(eMouseButton_WheelUp)] = true;
                        impl->m_mouseButton[static_cast<size_t>(eMouseButton_WheelDown)] = false;
                    } else if (sdlEvent.wheel.y < 0) {
                        impl->m_mouseButton[static_cast<size_t>(eMouseButton_WheelUp)] = false;
                        impl->m_mouseButton[static_cast<size_t>(eMouseButton_WheelDown)] = true;
                    }
                    break;
                default:
                    break;
                }
            });

            return handle;
        }
        cVector2l GetAbsPosition(InternalInputMouseHandle& handle) {
            auto* impl = static_cast<InternalInputMouseImpl*>(handle.Get());
            return impl->m_mousePosition;
        }

        cVector2l GetRelPosition(InternalInputMouseHandle& handle) {
            auto* impl = static_cast<InternalInputMouseImpl*>(handle.Get());
            return impl->m_mouseRelPosition;
        }

        window::internal::WindowInternalEvent::Handler& GetWindowEventHandle(InternalInputMouseHandle& handle) {
            auto* impl = static_cast<InternalInputMouseImpl*>(handle.Get());
            return impl->m_windowEventHandle;
        }
    } // namespace mouse

    namespace keyboard {
        struct InternalInputKeyboardImpl {
            window::internal::WindowInternalEvent::Handler m_windowEventHandle;
            IUpdateEventLoop::UpdateEvent::Handler m_postUpdate;
            IUpdateEventLoop::UpdateEvent::Handler m_preUpdate;
            std::mutex m_mutex;
            folly::small_vector<TextPress, 256> m_queuedCharacter;
            folly::small_vector<TextPress, 256> m_stagingCharacter;
            folly::small_vector<KeyPress, 256> m_queuedPresses;
            folly::small_vector<KeyPress, 256> m_stagingPresses;
            KeyPressEvent m_keyPressEvent;
            std::array<bool, static_cast<size_t>(eKey::eKey_LastEnum)> m_keyPressState = { { 0 } };
        };

        static eKey SDLToKey(uint32_t key) {
            switch (key) {
            case SDLK_BACKSPACE:
                return eKey_BackSpace;
            case SDLK_TAB:
                return eKey_Tab;
            case SDLK_CLEAR:
                return eKey_Clear;
            case SDLK_RETURN:
                return eKey_Return;
            case SDLK_PAUSE:
                return eKey_Pause;
            case SDLK_ESCAPE:
                return eKey_Escape;
            case SDLK_SPACE:
                return eKey_Space;
            case SDLK_EXCLAIM:
                return eKey_Exclaim;
            case SDLK_QUOTEDBL:
                return eKey_QuoteDouble;
            case SDLK_HASH:
                return eKey_Hash;
            case SDLK_DOLLAR:
                return eKey_Dollar;
            case SDLK_AMPERSAND:
                return eKey_Ampersand;
            case SDLK_QUOTE:
                return eKey_Quote;
            case SDLK_LEFTPAREN:
                return eKey_LeftParen;
            case SDLK_RIGHTPAREN:
                return eKey_RightParen;
            case SDLK_ASTERISK:
                return eKey_Asterisk;
            case SDLK_PLUS:
                return eKey_Plus;
            case SDLK_COMMA:
                return eKey_Comma;
            case SDLK_MINUS:
                return eKey_Minus;
            case SDLK_PERIOD:
                return eKey_Period;
            case SDLK_SLASH:
                return eKey_Slash;
            case SDLK_0:
                return eKey_0;
            case SDLK_1:
                return eKey_1;
            case SDLK_2:
                return eKey_2;
            case SDLK_3:
                return eKey_3;
            case SDLK_4:
                return eKey_4;
            case SDLK_5:
                return eKey_5;
            case SDLK_6:
                return eKey_6;
            case SDLK_7:
                return eKey_7;
            case SDLK_8:
                return eKey_8;
            case SDLK_9:
                return eKey_9;
            case SDLK_COLON:
                return eKey_Colon;
            case SDLK_SEMICOLON:
                return eKey_SemiColon;
            case SDLK_LESS:
                return eKey_Less;
            case SDLK_EQUALS:
                return eKey_Equals;
            case SDLK_GREATER:
                return eKey_Greater;
            case SDLK_QUESTION:
                return eKey_Question;
            case SDLK_AT:
                return eKey_At;
            case SDLK_LEFTBRACKET:
                return eKey_LeftBracket;
            case SDLK_BACKSLASH:
                return eKey_BackSlash;
            case SDLK_RIGHTBRACKET:
                return eKey_RightBracket;
            case SDLK_CARET:
                return eKey_Caret;
            case SDLK_UNDERSCORE:
                return eKey_Underscore;
            case SDLK_BACKQUOTE:
                return eKey_BackSlash;
            case SDLK_a:
                return eKey_A;
            case SDLK_b:
                return eKey_B;
            case SDLK_c:
                return eKey_C;
            case SDLK_d:
                return eKey_D;
            case SDLK_e:
                return eKey_E;
            case SDLK_f:
                return eKey_F;
            case SDLK_g:
                return eKey_G;
            case SDLK_h:
                return eKey_H;
            case SDLK_i:
                return eKey_I;
            case SDLK_j:
                return eKey_J;
            case SDLK_k:
                return eKey_K;
            case SDLK_l:
                return eKey_L;
            case SDLK_m:
                return eKey_M;
            case SDLK_n:
                return eKey_N;
            case SDLK_o:
                return eKey_O;
            case SDLK_p:
                return eKey_P;
            case SDLK_q:
                return eKey_Q;
            case SDLK_r:
                return eKey_R;
            case SDLK_s:
                return eKey_S;
            case SDLK_t:
                return eKey_T;
            case SDLK_u:
                return eKey_U;
            case SDLK_v:
                return eKey_V;
            case SDLK_w:
                return eKey_W;
            case SDLK_x:
                return eKey_X;
            case SDLK_y:
                return eKey_Y;
            case SDLK_z:
                return eKey_Z;
            case SDLK_DELETE:
                return eKey_Delete;
            case SDLK_KP_0:
                return eKey_KP_0;
            case SDLK_KP_1:
                return eKey_KP_1;
            case SDLK_KP_2:
                return eKey_KP_2;
            case SDLK_KP_3:
                return eKey_KP_3;
            case SDLK_KP_4:
                return eKey_KP_4;
            case SDLK_KP_5:
                return eKey_KP_5;
            case SDLK_KP_6:
                return eKey_KP_6;
            case SDLK_KP_7:
                return eKey_KP_7;
            case SDLK_KP_8:
                return eKey_KP_8;
            case SDLK_KP_9:
                return eKey_KP_9;
            case SDLK_KP_PERIOD:
                return eKey_KP_Period;
            case SDLK_KP_DIVIDE:
                return eKey_KP_Divide;
            case SDLK_KP_MULTIPLY:
                return eKey_KP_Multiply;
            case SDLK_KP_MINUS:
                return eKey_KP_Minus;
            case SDLK_KP_PLUS:
                return eKey_KP_Plus;
            case SDLK_KP_ENTER:
                return eKey_KP_Enter;
            case SDLK_KP_EQUALS:
                return eKey_KP_Equals;
            case SDLK_UP:
                return eKey_Up;
            case SDLK_DOWN:
                return eKey_Down;
            case SDLK_RIGHT:
                return eKey_Right;
            case SDLK_LEFT:
                return eKey_Left;
            case SDLK_INSERT:
                return eKey_Insert;
            case SDLK_HOME:
                return eKey_Home;
            case SDLK_END:
                return eKey_End;
            case SDLK_PAGEUP:
                return eKey_PageUp;
            case SDLK_PAGEDOWN:
                return eKey_PageDown;
            case SDLK_F1:
                return eKey_F1;
            case SDLK_F2:
                return eKey_F2;
            case SDLK_F3:
                return eKey_F3;
            case SDLK_F4:
                return eKey_F4;
            case SDLK_F5:
                return eKey_F5;
            case SDLK_F6:
                return eKey_F6;
            case SDLK_F7:
                return eKey_F7;
            case SDLK_F8:
                return eKey_F8;
            case SDLK_F9:
                return eKey_F9;
            case SDLK_F10:
                return eKey_F10;
            case SDLK_F11:
                return eKey_F11;
            case SDLK_F12:
                return eKey_F12;
            case SDLK_F13:
                return eKey_F13;
            case SDLK_F14:
                return eKey_F14;
            case SDLK_F15:
                return eKey_F15;
            case SDLK_NUMLOCKCLEAR:
                return eKey_NumLock;
            case SDLK_SCROLLLOCK:
                return eKey_ScrollLock;
            case SDLK_LGUI:
                return eKey_LeftSuper;
            case SDLK_RGUI:
                return eKey_RightSuper;
            case SDLK_PRINTSCREEN:
                return eKey_Print;
            case SDLK_CAPSLOCK:
                return eKey_CapsLock;
            case SDLK_RSHIFT:
                return eKey_RightShift;
            case SDLK_LSHIFT:
                return eKey_LeftShift;
            case SDLK_RCTRL:
                return eKey_RightCtrl;
            case SDLK_LCTRL:
                return eKey_LeftCtrl;
            case SDLK_RALT:
                return eKey_RightAlt;
            case SDLK_LALT:
                return eKey_LeftAlt;
            case SDLK_MODE:
                return eKey_Mode;
            case SDLK_HELP:
                return eKey_Help;
            case SDLK_SYSREQ:
                return eKey_SysReq;
            case SDLK_MENU:
                return eKey_Menu;
            case SDLK_POWER:
                return eKey_Power;
            }
            return eKey_LastEnum;
        };
        const std::span<TextPress> Characters(const InternalInputKeyboardHandle& handle) {
            auto* impl = static_cast<InternalInputKeyboardImpl*>(handle.Get());
            return impl->m_stagingCharacter;
        }
        const std::span<KeyPress> KeyPresses(const InternalInputKeyboardHandle& handle) {
            auto* impl = static_cast<InternalInputKeyboardImpl*>(handle.Get());
            return impl->m_stagingPresses;
        }
        bool IsKeyPressed(const InternalInputKeyboardHandle& handle, eKey key) {
            auto* impl = static_cast<InternalInputKeyboardImpl*>(handle.Get());
            return impl->m_keyPressState[static_cast<size_t>(key)];
        }

        void ConnectKeyPressEvent(const InternalInputKeyboardHandle& handle, KeyPressEvent::Handler& handler) {
            auto* impl = static_cast<InternalInputKeyboardImpl*>(handle.Get());
            handler.Connect(impl->m_keyPressEvent);
        }

        InternalInputKeyboardHandle Initialize() {
            auto ptr = InternalInputKeyboardHandle::Ptr(new InternalInputKeyboardImpl(), [](void* ptr) {
                auto impl = static_cast<InternalInputKeyboardImpl*>(ptr);
                impl->m_windowEventHandle.Disconnect();
                delete impl;
            });
            InternalInputKeyboardHandle handle = InternalInputKeyboardHandle(std::move(ptr));
            auto* impl = static_cast<InternalInputKeyboardImpl*>(handle.Get());

            impl->m_postUpdate = IUpdateEventLoop::UpdateEvent::Handler([impl](float afx) {
                // we've processed the presses, so clear them
                impl->m_stagingPresses.clear();
                impl->m_stagingCharacter.clear();
            });
            impl->m_preUpdate = IUpdateEventLoop::UpdateEvent::Handler([impl](float afx) {
                // move queued presses to staging for processing by the main thread
                std::lock_guard<std::mutex> lock(impl->m_mutex);
                for (auto itKey = impl->m_queuedPresses.rbegin(); itKey != impl->m_queuedPresses.rend(); ++itKey) {
                    impl->m_stagingPresses.push_back(*itKey);
                }
                for (auto itKey = impl->m_queuedCharacter.rbegin(); itKey != impl->m_queuedCharacter.rend(); ++itKey) {
                    impl->m_stagingCharacter.push_back(*itKey);
                }
                impl->m_queuedCharacter.clear();
                impl->m_queuedPresses.clear();
            });

            if (auto updateLoop = Interface<IUpdateEventLoop>::Get()) {
                updateLoop->Subscribe(BroadcastEvent::PostUpdate, impl->m_postUpdate);
                updateLoop->Subscribe(BroadcastEvent::PreUpdate, impl->m_preUpdate);
            } else {
                ASSERT(false && "No update loop found");
            }

            impl->m_windowEventHandle = window::internal::WindowInternalEvent::Handler([impl](hpl::window::InternalEvent& internalEvent) {
                auto& event = internalEvent.m_sdlEvent;
                switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    {
                        const bool isPressed = event.key.state == SDL_PRESSED;
                        eKeyModifier modifier = static_cast<eKeyModifier>(((event.key.keysym.mod & KMOD_SHIFT) ? eKeyModifier_Shift : eKeyModifier_None) |
                            ((event.key.keysym.mod & KMOD_CTRL) ? eKeyModifier_Ctrl : eKeyModifier_None) |
                            ((event.key.keysym.mod & KMOD_ALT) ? eKeyModifier_Alt : eKeyModifier_None));
                        auto key = SDLToKey(event.key.keysym.sym);
                        if (key != eKey_LastEnum) {
                            std::lock_guard<std::mutex> lock(impl->m_mutex);
                            KeyPress press = { key, modifier, isPressed };
                            impl->m_keyPressEvent.Signal(press);
                            impl->m_queuedPresses.push_back(press);
                            impl->m_keyPressState[static_cast<size_t>(key)] = isPressed;
                        }
                        break;
                    }
                case SDL_TEXTINPUT:
                    {
                        std::lock_guard<std::mutex> lock(impl->m_mutex);
                        eKeyModifier modifier = static_cast<eKeyModifier>(((event.key.keysym.mod & KMOD_SHIFT) ? eKeyModifier_Shift : eKeyModifier_None) |
                            ((event.key.keysym.mod & KMOD_CTRL) ? eKeyModifier_Ctrl : eKeyModifier_None) |
                            ((event.key.keysym.mod & KMOD_ALT) ? eKeyModifier_Alt : eKeyModifier_None));
                        impl->m_queuedCharacter.push_back({event.text.text[0],modifier  });
                        break;
                    }
                }
            });
            return handle;
        }

        window::internal::WindowInternalEvent::Handler& GetWindowEventHandle(InternalInputKeyboardHandle& handle) {
            auto* impl = static_cast<InternalInputKeyboardImpl*>(handle.Get());
            return impl->m_windowEventHandle;
        }

    } // namespace keyboard

} // namespace hpl::input::internal
