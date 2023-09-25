#pragma once

#include <engine/RTTI.h>

namespace hpl::input {
    class InputManagerInterface {
        HPL_RTTI_CLASS(InputManagerInterface, "{63d2ba2c-9ef8-463b-a044-43281b11fa9d}")
    public:
        virtual ~InputManagerInterface() = default;
    };
}