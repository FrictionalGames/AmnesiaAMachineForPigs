#pragma once

#include <engine/RTTI.h>

namespace hpl::input {

    class BaseInputDevice {
        HPL_RTTI_CLASS(BaseInputDevice, "{9e04fa77-3045-4693-ba5d-be7d117a9122}")
    public:
        BaseInputDevice() {}
        virtual ~BaseInputDevice() {}
    };
}