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

#include "engine/UpdateEventLoop.h"
#include "graphics/ForgeRenderer.h"
#include "windowing/NativeWindow.h"
#include <cstdint>
#include <engine/RTTI.h>
#include <functional>
#include <memory>
#include <input/InputManager.h>

#include "Common_3/Utilities/Interfaces/IThread.h"
#include <FixPreprocessor.h>

namespace hpl {
    class PrimaryViewport;
    // Bootstrap the engine and provide access to the core systems
    class Bootstrap {
    public:
        struct BootstrapConfiguration final {
        public:
            BootstrapConfiguration() {}
            window::WindowStyle m_windowStyle = window::WindowStyle::WindowStyleNone;
        };

        Bootstrap();
        ~Bootstrap();

        void Initialize(BootstrapConfiguration configuration = BootstrapConfiguration{ } );
        void Run(std::function<void()> handler);
        void Shutdown();
    private:
        static void BootstrapThreadHandler(void* _userData);

        std::unique_ptr<PrimaryViewport> m_primaryViewport;
        hpl::ForgeRenderer m_renderer;

        UpdateEventLoop m_updateEventLoop;
        input::InputManager m_inputManager;
        window::NativeWindowWrapper m_window;
        ThreadHandle m_thread;
        std::function<void()> m_handler;
        std::atomic_bool m_isRunning;
    };
}
