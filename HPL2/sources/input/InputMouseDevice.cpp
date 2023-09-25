#include <input/InputMouseDevice.h>

namespace hpl::input {
    namespace detail {
        std::string_view MouseButtonToString(eMouseButton button) {
            switch (button) {
            case eMouseButton_Left:
                return "LeftMouse";
            case eMouseButton_Middle:
                return "MiddleMouse";
            case eMouseButton_Right:
                return "RightMouse";
            case eMouseButton_WheelUp:
                return "WheelUp";
            case eMouseButton_WheelDown:
                return "WheelDown";
            case eMouseButton_Button6:
                return "Mouse6";
            case eMouseButton_Button7:
                return "Mouse7";
            case eMouseButton_Button8:
                return "Mouse8";
            case eMouseButton_Button9:
                return "Mouse9";
            default:
                break;
            }
            return "Unknown";
        }

        eMouseButton StringToMouseButton(const std::string_view& button) {
            if (button == "LeftMouse") {
                return eMouseButton_Left;
            }
            if (button == "MiddleMouse") {
                return eMouseButton_Middle;
            }
            if (button == "RightMouse") {
                return eMouseButton_Right;
            }
            if (button == "WheelUp") {
                return eMouseButton_WheelUp;
            }
            if (button == "WheelDown") {
                return eMouseButton_WheelDown;
            }
            if (button == "Mouse6") {
                return eMouseButton_Button6;
            }
            if (button == "Mouse7") {
                return eMouseButton_Button7;
            }
            if (button == "Mouse8") {
                return eMouseButton_Button8;
            }
            if (button == "Mouse9") {
                return eMouseButton_Button9;
            }
            return eMouseButton_LastEnum;
        }

    } // namespace details
} // namespace hpl::input