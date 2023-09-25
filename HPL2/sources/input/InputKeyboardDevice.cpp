#include "input/InputTypes.h"
#include <input/InputKeyboardDevice.h>

namespace hpl::input {
    namespace detail {
        char KeyToCharacter(eKey key, bool capitialize) {
            switch (key) {
            case eKey_A:
                return capitialize ? 'A' : 'a';
                break;
            case eKey_B:
                return capitialize ? 'B' : 'b';
                break;
            case eKey_C:
                return capitialize ? 'C' : 'c';
                break;
            case eKey_D:
                return capitialize ? 'D' : 'd';
                break;
            case eKey_E:
                return capitialize ? 'E' : 'e';
                break;
            case eKey_F:
                return capitialize ? 'F' : 'f';
                break;
            case eKey_G:
                return capitialize ? 'G' : 'g';
                break;
            case eKey_H:
                return capitialize ? 'H' : 'h';
                break;
            case eKey_I:
                return capitialize ? 'I' : 'i';
                break;
            case eKey_J:
                return capitialize ? 'J' : 'j';
                break;
            case eKey_K:
                return capitialize ? 'K' : 'k';
                break;
            case eKey_L:
                return capitialize ? 'L' : 'l';
                break;
            case eKey_M:
                return capitialize ? 'M' : 'm';
                break;
            case eKey_N:
                return capitialize ? 'N' : 'n';
                break;
            case eKey_O:
                return capitialize ? 'O' : 'o';
                break;
            case eKey_P:
                return capitialize ? 'P' : 'p';
                break;
            case eKey_Q:
                return capitialize ? 'Q' : 'q';
                break;
            case eKey_R:
                return capitialize ? 'R' : 'r';
                break;
            case eKey_S:
                return capitialize ? 'S' : 's';
                break;
            case eKey_T:
                return capitialize ? 'T' : 't';
                break;
            case eKey_U:
                return capitialize ? 'U' : 'u';
                break;
            case eKey_V:
                return capitialize ? 'V' : 'v';
                break;
            case eKey_W:
                return capitialize ? 'W' : 'w';
            case eKey_X:
                return capitialize ? 'X' : 'x';
            case eKey_Y:
                return capitialize ? 'Y' : 'y';
            case eKey_Z:
                return capitialize ? 'Z' : 'z';
            case eKey_0:
                return '0';
            case eKey_1:
                return '1';
            case eKey_2:
                return '2';
            case eKey_3:
                return '3';
            case eKey_4:
                return '4';
            case eKey_5:
                return '5';
            case eKey_6:
                return '6';
            case eKey_7:
                return '7';
            case eKey_8:
                return '8';
            case eKey_9:
                return '9';
                
            default:

                break;
            }
            return 0;
        }
        std::string_view KeyToString(eKey key) {
            switch(key)
            {
                case eKey_BackSpace: return "BackSpace";
                case eKey_Tab: return "Tab";
                case eKey_Clear: return "Clear";
                case eKey_Return: return "Return";
                case eKey_Pause: return "Pause";
                case eKey_Escape: return "Escape";
                case eKey_Space: return "Space";
                case eKey_Exclaim: return "Exclaim";
                case eKey_QuoteDouble: return "DblQoute";
                case eKey_Hash: return "Hash";
                case eKey_Dollar: return "Dollar";
                case eKey_Ampersand: return "Ampersand";
                case eKey_Quote: return "Quote";
                case eKey_LeftParen: return "LeftParent";
                case eKey_RightParen: return "RightParent";
                case eKey_Asterisk: return "Asterisk";
                case eKey_Plus: return "Plus";
                case eKey_Comma: return "Comma";
                case eKey_Minus: return "Minus";
                case eKey_Period: return "Period";
                case eKey_Slash: return "Slash";
                case eKey_0: return "0";
                case eKey_1: return "1";
                case eKey_2: return "2";
                case eKey_3: return "3";
                case eKey_4: return "4";
                case eKey_5: return "5";
                case eKey_6: return "6";
                case eKey_7: return "7";
                case eKey_8: return "8";
                case eKey_9: return "9";
                case eKey_Colon: return "Colon";
                case eKey_SemiColon: return "SemiColon";
                case eKey_Less: return "Less";
                case eKey_Equals: return "Equals";
                case eKey_Greater: return "Greater";
                case eKey_Question: return "Question";
                case eKey_At: return "At";
                case eKey_LeftBracket: return "LeftBracket";
                case eKey_BackSlash: return "BackSlash";
                case eKey_RightBracket: return "RightBracket";
                case eKey_Caret: return "Caret";
                case eKey_Underscore: return "Underscore";
                case eKey_BackQuote: return "BackQuote";
                case eKey_A: return "A";
                case eKey_B: return "B";
                case eKey_C: return "C";
                case eKey_D: return "D";
                case eKey_E: return "E";
                case eKey_F: return "F";
                case eKey_G: return "G";
                case eKey_H: return "H";
                case eKey_I: return "I";
                case eKey_J: return "J";
                case eKey_K: return "K";
                case eKey_L: return "L";
                case eKey_M: return "M";
                case eKey_N: return "N";
                case eKey_O: return "O";
                case eKey_P: return "P";
                case eKey_Q: return "Q";
                case eKey_R: return "R";
                case eKey_S: return "S";
                case eKey_T: return "T";
                case eKey_U: return "U";
                case eKey_V: return "V";
                case eKey_W: return "W";
                case eKey_X: return "X";
                case eKey_Y: return "Y";
                case eKey_Z: return "Z";
                case eKey_Delete: return "Delete";
                case eKey_KP_0: return "KP_0";
                case eKey_KP_1: return "KP_1";
                case eKey_KP_2: return "KP_2";
                case eKey_KP_3: return "KP_3";
                case eKey_KP_4: return "KP_4";
                case eKey_KP_5: return "KP_5";
                case eKey_KP_6: return "KP_6";
                case eKey_KP_7: return "KP_7";
                case eKey_KP_8: return "KP_8";
                case eKey_KP_9: return "KP_9";
                case eKey_KP_Period: return "KP_Period";
                case eKey_KP_Divide: return "KP_Divide";
                case eKey_KP_Multiply: return "KP_Multiply";
                case eKey_KP_Minus: return "KP_Minus";
                case eKey_KP_Plus: return "KP_Plus";
                case eKey_KP_Enter: return "KP_Enter";
                case eKey_KP_Equals: return "KP_Equals";
                case eKey_Up: return "Up";
                case eKey_Down: return "Down";
                case eKey_Right: return "Right";
                case eKey_Left: return "Left";
                case eKey_Insert: return "Insert";
                case eKey_Home: return "Home";
                case eKey_End: return "End";
                case eKey_PageUp: return "PageUp";
                case eKey_PageDown: return "PageDown";
                case eKey_F1: return "F1";
                case eKey_F2: return "F2";
                case eKey_F3: return "F3";
                case eKey_F4: return "F4";
                case eKey_F5: return "F5";
                case eKey_F6: return "F6";
                case eKey_F7: return "F7";
                case eKey_F8: return "F8";
                case eKey_F9: return "F9";
                case eKey_F10: return "F10";
                case eKey_F11: return "F11";
                case eKey_F12: return "F12";
                case eKey_F13: return "F13";
                case eKey_F14: return "F14";
                case eKey_F15: return "F15";
                case eKey_NumLock: return "NumLock";
                case eKey_CapsLock: return "CapsLock";
                case eKey_ScrollLock: return "ScrollLock";
                case eKey_RightShift: return "RightShift";
                case eKey_LeftShift: return "LeftShift";
                case eKey_RightCtrl: return "RightControl";
                case eKey_LeftCtrl: return "LeftControl";
                case eKey_RightAlt: return "RightAlt";
                case eKey_LeftAlt: return "LeftAlt";
                case eKey_RightMeta: return "RightMeta";
                case eKey_LeftMeta: return "LeftMeta";
                case eKey_LeftSuper: return "LeftSuper";
                case eKey_RightSuper: return "RightSuper";
                case eKey_Mode: return "Mode";
                case eKey_Help: return "Help";
                case eKey_Print: return "Print";
                case eKey_SysReq: return "SysReq";
                case eKey_Break: return "Break";
                case eKey_Menu: return "Menu";
                case eKey_Power: return "Power";
                case eKey_Euro: return "Euro";
                case eKey_None: return "None";
                case eKey_World_0: return "World_0";
                case eKey_World_1: return "World_1";
                case eKey_World_2: return "World_2";
                case eKey_World_3: return "World_3";
                case eKey_World_4: return "World_4";
                case eKey_World_5: return "World_5";
                case eKey_World_6: return "World_6";
                case eKey_World_7: return "World_7";
                case eKey_World_8: return "World_8";
                case eKey_World_9: return "World_9";
                case eKey_World_10: return "World_10";
                case eKey_World_11: return "World_11";
                case eKey_World_12: return "World_12";
                case eKey_World_13: return "World_13";
                case eKey_World_14: return "World_14";
                case eKey_World_15: return "World_15";
                case eKey_World_16: return "World_16";
                case eKey_World_17: return "World_17";
                case eKey_World_18: return "World_18";
                case eKey_World_19: return "World_19";
                case eKey_World_20: return "World_20";
                case eKey_World_21: return "World_21";
                case eKey_World_22: return "World_22";
                case eKey_World_23: return "World_23";
                case eKey_World_24: return "World_24";
                case eKey_World_25: return "World_25";
                case eKey_World_26: return "World_26";
                case eKey_World_27: return "World_27";
                case eKey_World_28: return "World_28";
                case eKey_World_29: return "World_29";
                case eKey_World_30: return "World_30";
                case eKey_World_31: return "World_31";
                case eKey_World_32: return "World_32";
                case eKey_World_33: return "World_33";
                case eKey_World_34: return "World_34";
                case eKey_World_35: return "World_35";
                case eKey_World_36: return "World_36";
                case eKey_World_37: return "World_37";
                case eKey_World_38: return "World_38";
                case eKey_World_39: return "World_39";
                case eKey_World_40: return "World_40";
                case eKey_World_41: return "World_41";
                case eKey_World_42: return "World_42";
                case eKey_World_43: return "World_43";
                case eKey_World_44: return "World_44";
                case eKey_World_45: return "World_45";
                case eKey_World_46: return "World_46";
                case eKey_World_47: return "World_47";
                case eKey_World_48: return "World_48";
                case eKey_World_49: return "World_49";
                case eKey_World_50: return "World_50";
                case eKey_World_51: return "World_51";
                case eKey_World_52: return "World_52";
                case eKey_World_53: return "World_53";
                case eKey_World_54: return "World_54";
                case eKey_World_55: return "World_55";
                case eKey_World_56: return "World_56";
                case eKey_World_57: return "World_57";
                case eKey_World_58: return "World_58";
                case eKey_World_59: return "World_59";
                case eKey_World_60: return "World_60";
                case eKey_World_61: return "World_61";
                case eKey_World_62: return "World_62";
                case eKey_World_63: return "World_63";
                case eKey_World_64: return "World_64";
                case eKey_World_65: return "World_65";
                case eKey_World_66: return "World_66";
                case eKey_World_67: return "World_67";
                case eKey_World_68: return "World_68";
                case eKey_World_69: return "World_69";
                case eKey_World_70: return "World_70";
                case eKey_World_71: return "World_71";
                case eKey_World_72: return "World_72";
                case eKey_World_73: return "World_73";
                case eKey_World_74: return "World_74";
                case eKey_World_75: return "World_75";
                case eKey_World_76: return "World_76";
                case eKey_World_77: return "World_77";
                case eKey_World_78: return "World_78";
                case eKey_World_79: return "World_79";
                case eKey_World_80: return "World_80";
                case eKey_World_81: return "World_81";
                case eKey_World_82: return "World_82";
                case eKey_World_83: return "World_83";
                case eKey_World_84: return "World_84";
                case eKey_World_85: return "World_85";
                case eKey_World_86: return "World_86";
                case eKey_World_87: return "World_87";
                case eKey_World_88: return "World_88";
                case eKey_World_89: return "World_89";
                case eKey_World_90: return "World_90";
                case eKey_World_91: return "World_91";
                case eKey_World_92: return "World_92";
                case eKey_World_93: return "World_93";
                case eKey_World_94: return "World_94";
                case eKey_World_95: return "World_95";
                default:
                    break;
            }

            return "Unknown";
        }
    }

    // Modifier operator|(Modifier lhs, Modifier rhs) {
    //     return static_cast<Modifier>(static_cast<int>(lhs) | static_cast<int>(rhs));
    // }
    // bool operator&(Modifier lhs, Modifier rhs) {
    //     return static_cast<int>(lhs) & static_cast<int>(rhs);
    // }
} // namespace hpl::input