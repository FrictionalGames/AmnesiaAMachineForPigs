#pragma once

#include "input/InputTypes.h"
#include <windowing/NativeWindow.h>
#include <input/BaseInputDevice.h>
#include <system/HandleWrapper.h>

#include <cstdint>
#include <span>

namespace hpl::input {

    // enum class Key {
    //     KeyBackSpace,
    //     KeyTab,
    //     KeyClear,
    //     KeyReturn,
    //     KeyPause,
    //     KeyEscape,
    //     KeySpace,
    //     KeyExclaim,
    //     KeyQuoteDouble,
    //     KeyHash,
    //     KeyDollar,
    //     KeyAmpersand,
    //     KeyQuote,
    //     KeyLeftParen,
    //     KeyRightParen,
    //     KeyAsterisk,
    //     KeyPlus,
    //     KeyComma,
    //     KeyMinus,
    //     KeyPeriod,
    //     KeySlash,
    //     Key0,
    //     Key1,
    //     Key2,
    //     Key3,
    //     Key4,
    //     Key5,
    //     Key6,
    //     Key7,
    //     Key8,
    //     Key9,
    //     KeyColon,
    //     KeySemiColon,
    //     KeyLess,
    //     KeyEquals,
    //     KeyGreater,
    //     KeyQuestion,
    //     KeyAt,
    //     KeyLeftBracket,
    //     KeyBackSlash,
    //     KeyRightBracket,
    //     KeyCaret,
    //     KeyUnderscore,
    //     KeyBackQuote,
    //     KeyA,
    //     KeyB,
    //     KeyC,
    //     KeyD,
    //     KeyE,
    //     KeyF,
    //     KeyG,
    //     KeyH,
    //     KeyI,
    //     KeyJ,
    //     KeyK,
    //     KeyL,
    //     KeyM,
    //     KeyN,
    //     KeyO,
    //     KeyP,
    //     KeyQ,
    //     KeyR,
    //     KeyS,
    //     KeyT,
    //     KeyU,
    //     KeyV,
    //     KeyW,
    //     KeyX,
    //     KeyY,
    //     KeyZ,
    //     KeyDelete,
    //     KeyKP_0,
    //     KeyKP_1,
    //     KeyKP_2,
    //     KeyKP_3,
    //     KeyKP_4,
    //     KeyKP_5,
    //     KeyKP_6,
    //     KeyKP_7,
    //     KeyKP_8,
    //     KeyKP_9,
    //     KeyKP_Period,
    //     KeyKP_Divide,
    //     KeyKP_Multiply,
    //     KeyKP_Minus,
    //     KeyKP_Plus,
    //     KeyKP_Enter,
    //     KeyKP_Equals,
    //     KeyUp,
    //     KeyDown,
    //     KeyRight,
    //     KeyLeft,
    //     KeyInsert,
    //     KeyHome,
    //     KeyEnd,
    //     KeyPageUp,
    //     KeyPageDown,
    //     KeyF1,
    //     KeyF2,
    //     KeyF3,
    //     KeyF4,
    //     KeyF5,
    //     KeyF6,
    //     KeyF7,
    //     KeyF8,
    //     KeyF9,
    //     KeyF10,
    //     KeyF11,
    //     KeyF12,
    //     KeyF13,
    //     KeyF14,
    //     KeyF15,
    //     KeyNumLock,
    //     KeyCapsLock,
    //     KeyScrollLock,
    //     KeyRightShift,
    //     KeyLeftShift,
    //     KeyRightCtrl,
    //     KeyLeftCtrl,
    //     KeyRightAlt,
    //     KeyLeftAlt,
    //     KeyRightMeta,
    //     KeyLeftMeta,
    //     KeyLeftSuper,
    //     KeyRightSuper,
    //     KeyMode,
    //     KeyHelp,
    //     KeyPrint,
    //     KeySysReq,
    //     KeyBreak,
    //     KeyMenu,
    //     KeyPower,
    //     KeyEuro,
    //     KeyNone,
    //     KeyWorld_0,
    //     KeyWorld_1,
    //     KeyWorld_2,
    //     KeyWorld_3,
    //     KeyWorld_4,
    //     KeyWorld_5,
    //     KeyWorld_6,
    //     KeyWorld_7,
    //     KeyWorld_8,
    //     KeyWorld_9,
    //     KeyWorld_10,
    //     KeyWorld_11,
    //     KeyWorld_12,
    //     KeyWorld_13,
    //     KeyWorld_14,
    //     KeyWorld_15,
    //     KeyWorld_16,
    //     KeyWorld_17,
    //     KeyWorld_18,
    //     KeyWorld_19,
    //     KeyWorld_20,
    //     KeyWorld_21,
    //     KeyWorld_22,
    //     KeyWorld_23,
    //     KeyWorld_24,
    //     KeyWorld_25,
    //     KeyWorld_26,
    //     KeyWorld_27,
    //     KeyWorld_28,
    //     KeyWorld_29,
    //     KeyWorld_30,
    //     KeyWorld_31,
    //     KeyWorld_32,
    //     KeyWorld_33,
    //     KeyWorld_34,
    //     KeyWorld_35,
    //     KeyWorld_36,
    //     KeyWorld_37,
    //     KeyWorld_38,
    //     KeyWorld_39,
    //     KeyWorld_40,
    //     KeyWorld_41,
    //     KeyWorld_42,
    //     KeyWorld_43,
    //     KeyWorld_44,
    //     KeyWorld_45,
    //     KeyWorld_46,
    //     KeyWorld_47,
    //     KeyWorld_48,
    //     KeyWorld_49,
    //     KeyWorld_50,
    //     KeyWorld_51,
    //     KeyWorld_52,
    //     KeyWorld_53,
    //     KeyWorld_54,
    //     KeyWorld_55,
    //     KeyWorld_56,
    //     KeyWorld_57,
    //     KeyWorld_58,
    //     KeyWorld_59,
    //     KeyWorld_60,
    //     KeyWorld_61,
    //     KeyWorld_62,
    //     KeyWorld_63,
    //     KeyWorld_64,
    //     KeyWorld_65,
    //     KeyWorld_66,
    //     KeyWorld_67,
    //     KeyWorld_68,
    //     KeyWorld_69,
    //     KeyWorld_70,
    //     KeyWorld_71,
    //     KeyWorld_72,
    //     KeyWorld_73,
    //     KeyWorld_74,
    //     KeyWorld_75,
    //     KeyWorld_76,
    //     KeyWorld_77,
    //     KeyWorld_78,
    //     KeyWorld_79,
    //     KeyWorld_80,
    //     KeyWorld_81,
    //     KeyWorld_82,
    //     KeyWorld_83,
    //     KeyWorld_84,
    //     KeyWorld_85,
    //     KeyWorld_86,
    //     KeyWorld_87,
    //     KeyWorld_88,
    //     KeyWorld_89,
    //     KeyWorld_90,
    //     KeyWorld_91,
    //     KeyWorld_92,
    //     KeyWorld_93,
    //     KeyWorld_94,
    //     KeyWorld_95,

    //     LastEnum
    // };

    // enum class Modifier : uint8_t {
    //     None = 0,
    //     Ctrl = 0x1,
    //     Shift = 0x2,
    //     Alt = 0x4,
    // };
    namespace detail {
        static char KeyToCharacter(eKey key, bool capitialize);
        std::string_view KeyToString(eKey key);
    }

    struct KeyPress {
        eKey key;
        eKeyModifier modifier;
        bool pressed;
    };

    struct TextPress {
        char character;
        eKeyModifier modifier;
    };

    using KeyPressEvent = hpl::Event<KeyPress&>;

    namespace internal::keyboard {
        class InternalInputKeyboardHandle final {
            HPL_HANDLER_IMPL(InternalInputKeyboardHandle)
        };

        InternalInputKeyboardHandle Initialize();

        const std::span<TextPress> Characters(const InternalInputKeyboardHandle& handle);
        const std::span<KeyPress> KeyPresses(const InternalInputKeyboardHandle& handle);
        
        void ConnectKeyPressEvent(const InternalInputKeyboardHandle& handle, KeyPressEvent::Handler& handler);

        bool IsKeyPressed(const InternalInputKeyboardHandle& handle, eKey key);

        window::internal::WindowInternalEvent::Handler& GetWindowEventHandle(InternalInputKeyboardHandle& handle); // internal use only
    } // namespace internal::keyboard

    class InputKeyboardDevice final : public BaseInputDevice {
        HPL_RTTI_IMPL_CLASS(BaseInputDevice, InputKeyboardDevice, "{61161f8c-bd7c-431b-ae1f-2308bb50055e}")
    public:
		InputKeyboardDevice() = default;
        InputKeyboardDevice(internal::keyboard::InternalInputKeyboardHandle&& handle) :
            m_impl(std::move(handle)) {
        }
        InputKeyboardDevice(InputKeyboardDevice&& other)
            : m_impl(std::move(other.m_impl)) {
        }

        void operator=(InputKeyboardDevice&& other) {
            m_impl = std::move(other.m_impl);
        }

        inline const std::span<TextPress> Characters() const {
            return internal::keyboard::Characters(m_impl);
        }

        inline const std::span<KeyPress> KeyPresses() const {
            return internal::keyboard::KeyPresses(m_impl);
        }

        inline const bool IsKeyPressed(eKey key) const {
            return internal::keyboard::IsKeyPressed(m_impl, key);
        }

        inline void ConnectKeyPressEvent(KeyPressEvent::Handler& handler) {
            internal::keyboard::ConnectKeyPressEvent(m_impl, handler);
        }

        window::internal::WindowInternalEvent::Handler& GetWindowEventHandle() {
            return internal::keyboard::GetWindowEventHandle(m_impl);
        }

        eKeyModifier GetModifier() {
            return static_cast<eKeyModifier>(
                 ((IsKeyPressed(eKey_RightCtrl) || IsKeyPressed(eKey_LeftCtrl))? eKeyModifier_Ctrl : eKeyModifier::eKeyModifier_None)
                | ((IsKeyPressed(eKey_RightShift) || IsKeyPressed(eKey_LeftShift))? eKeyModifier_Shift : eKeyModifier::eKeyModifier_None)
                | ((IsKeyPressed(eKey_RightAlt) || IsKeyPressed(eKey_LeftAlt))? eKeyModifier_Alt : eKeyModifier::eKeyModifier_None));
        }

    private:
        internal::keyboard::InternalInputKeyboardHandle m_impl;
    };

} // namespace hpl::input