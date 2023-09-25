#pragma once

#include "engine/Interface.h"
#include "engine/RTTI.h"
#include <input/InputDeviceId.h>
#include <input/InputManagerInterface.h>

#include <memory>
#include <vector>

namespace hpl::input {
    class BaseInputDevice;
    class InputManager;

    class InputManager : public InputManagerInterface {
    public:
        InputManager();
        virtual ~InputManager();

        static constexpr InputDeviceID MouseDeviceID = InputDeviceID("Mouse");
        static constexpr InputDeviceID KeyboardDeviceID = InputDeviceID("Keyboard");

        struct InputEntry {
            InputDeviceID m_id;
            std::shared_ptr<BaseInputDevice> m_inputDevice;
        };

        std::shared_ptr<BaseInputDevice> GetDevice(InputDeviceID id);
        
        template<class TDevice>
        std::shared_ptr<TDevice> GetDevice(InputDeviceID id) {
            auto device = GetDevice(id);
            if(device && TypeInfo<TDevice>::IsType(*device.get())) {
                return std::static_pointer_cast<TDevice>(device);
            }
            return nullptr;
        }

        void Register(InputDeviceID id, std::shared_ptr<BaseInputDevice> inputDevice);
    private:
        std::vector<InputEntry> m_inputsDevices;
    };
    namespace detail {
        template<class TDevice>
        std::shared_ptr<TDevice> UtilityFetchDevice(InputDeviceID id) {
            input::InputManager* inputManager = nullptr;
            std::shared_ptr<TDevice> device;
            if((inputManager = Interface<input::InputManager>::Get()) && 
                (device = inputManager->GetDevice<TDevice>(id))) {
                return device;
            }
            return nullptr;
        }
    }
} // namespace hpl::input