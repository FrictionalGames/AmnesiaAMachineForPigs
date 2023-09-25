#include <input/InputManager.h>

namespace hpl::input {

    InputManager::InputManager() {
    }

    InputManager::~InputManager() {
    }

    void InputManager::Register(InputDeviceID id, std::shared_ptr<BaseInputDevice> inputDevice) {
        m_inputsDevices.push_back({id, inputDevice});
    }


    std::shared_ptr<BaseInputDevice> InputManager::GetDevice(InputDeviceID id) {
        for (auto& entry : m_inputsDevices) {
            if (entry.m_id == id) {
                return entry.m_inputDevice;
            }
        }
        return nullptr;
    }
}