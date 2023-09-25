#pragma once

#include <math/MathTypes.h>
#include <engine/windowing/WindowInterface.h>
#include <engine/RTTI.h>

enum class MouseButton {
    Left,
    Middle,
    Right,
    WheelUp,
    WheelDown,
    Button6,
    Button7,
    Button8,
    Button9,
    LastEnum
};

namespace hpl {
    class MouseInterface {

    public:
        virtual ~MouseInterface() = default;

        virtual bool ButtonIsDown(MouseButton mButton) = 0;
        
        virtual cVector2l GetAbsPosition() = 0;
        virtual cVector2l GetRelPosition() = 0;

        virtual WindowInterface::WindowInternalEvent::Handler& WindowInternalHandler() = 0;
    };
};