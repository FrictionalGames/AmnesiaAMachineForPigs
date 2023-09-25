/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "impl/LowLevelInputSDL.h"

#include "impl/GamepadSDL.h"
#include "impl/GamepadSDL2.h"
#include "impl/KeyboardSDL.h"
#include "impl/MouseSDL.h"
#include <engine/Event.h>
#include <engine/Interface.h>

#include "graphics/LowLevelGraphics.h"
#include "system/LowLevelSystem.h"

#include "engine/Engine.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"

#include <SDL2/SDL_events.h>
#include <windowing/NativeWindow.h>

namespace hpl {

    cLowLevelInputSDL::cLowLevelInputSDL()
        : mbQuitMessagePosted(false)
        , m_windowEventHandler(
              BroadcastEvent::PreUpdate,
              [&](auto& internalEvent) {
                  auto& event = internalEvent.m_sdlEvent;
                  // built-in SDL2 gamepad hotplug code
                  // this whole contract should be rewritten to allow clean adding/removing
                  // of controllers, instead of brute force rescanning
                  if (event.type == SDL_CONTROLLERDEVICEADDED) {
                      // sdlEvent.cdevice.which is the device #
                      cEngine::SetDeviceWasPlugged();
                  } else if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
                      // sdlEvent.cdevice.which is the instance # (not device #).
                      // instance # increases as devices are plugged and unplugged.
                      cEngine::SetDeviceWasRemoved();
                  }
#if defined(__APPLE__)
                  if (event.type == SDL_KEYDOWN) {
                      if (event.key.keysym.sym == SDLK_q && sdlEvent.key.keysym.mod & KMOD_GUI) {
                          mbQuitMessagePosted = true;
                      } else {
                          mlstEvents.push_back(sdlEvent);
                      }
                  } else
#endif
                      if (event.type == SDL_QUIT) {
                      mbQuitMessagePosted = true;
                  } else {
                      mlstEvents.push_back(event);
                  }
              },
              { [&]() {
                   mlstEvents.clear();
               },
                []() {
                },
                [](auto ev) {
                    return true;
                } }) {
        m_windowEventHandler.Connect(Interface<window::NativeWindowWrapper>::Get()->NativeInternalEvent());

        LockInput(true);
        RelativeMouse(false);
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    //-----------------------------------------------------------------------

    cLowLevelInputSDL::~cLowLevelInputSDL() {
    }

    void cLowLevelInputSDL::LockInput(bool abX) {
        if (auto* window = Interface<window::NativeWindowWrapper>::Get()) {
            if (abX) {
                window->GrabCursor();
            } else {
                window->ReleaseCursor();
            }
        }
    }

    void cLowLevelInputSDL::RelativeMouse(bool abX) {
        if (auto* window = Interface<window::NativeWindowWrapper>::Get()) {
            if (abX) {
                window->ConstrainCursor();
            } else {
                window->UnconstrainCursor();
            }
        }
    }

    void cLowLevelInputSDL::BeginInputUpdate() {
        // mlstEvents.clear();
        // m_windowHandler.Process();
    }

    void cLowLevelInputSDL::EndInputUpdate() {
    }

    void cLowLevelInputSDL::InitGamepadSupport() {
#if !SDL_VERSION_ATLEAST(2, 0, 0)
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
#endif
    }

    void cLowLevelInputSDL::DropGamepadSupport() {
#if !SDL_VERSION_ATLEAST(2, 0, 0)
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
#endif
    }

    int cLowLevelInputSDL::GetPluggedGamepadNum() {
#if USE_XINPUT
        return cGamepadXInput::GetNumConnected();
#else
        return SDL_NumJoysticks();
#endif
    }

    iMouse* cLowLevelInputSDL::CreateMouse() {
        return hplNew(cMouseSDL, (this));
    }

    iKeyboard* cLowLevelInputSDL::CreateKeyboard() {
        return hplNew(cKeyboardSDL, (this));
    }

    iGamepad* cLowLevelInputSDL::CreateGamepad(int alIndex) {
#if USE_SDL2
        return hplNew(cGamepadSDL2, (this, alIndex));
#else
        return hplNew(cGamepadSDL, (this, alIndex));
#endif
    }

    bool cLowLevelInputSDL::isQuitMessagePosted() {
        return mbQuitMessagePosted;
    }

    void cLowLevelInputSDL::resetQuitMessagePosted() {
        mbQuitMessagePosted = false;
    }

} // namespace hpl
