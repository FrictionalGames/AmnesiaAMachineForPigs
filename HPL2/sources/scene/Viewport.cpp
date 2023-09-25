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

#include "engine/Interface.h"
#include "graphics/Renderer.h"

#include "scene/Viewport.h"
#include "scene/Scene.h"
#include "scene/World.h"

#include <array>
#include <memory>

#include <folly/small_vector.h>

namespace hpl {

    namespace internal {
        static size_t m_id = 0;
        static folly::small_vector<size_t, cViewport::MaxViewportHandles> m_freelist;
        static std::mutex m_mutex;
    } // namespace internal

    cViewport::cViewport(cScene* apScene)
        : mpRenderSettings(std::make_unique<cRenderSettings>())
        , mpScene(apScene)
        , mbActive(true)
        , mbVisible(true)
        , mpWorld(nullptr)
        , mpCamera(nullptr)
        , mpRenderer(nullptr)
        , mpPostEffectComposite(nullptr)
        , mbIsListener(false) {

        std::lock_guard<std::mutex> lock(internal::m_mutex);
        if (internal::m_freelist.empty()) {
            ASSERT(internal::m_id < MaxViewportHandles && "MaxViewportHandles exceeded");
            m_handle = ++internal::m_id;
        } else {
            m_handle = internal::m_freelist.back();
            internal::m_freelist.pop_back();
        }

        m_updateEventHandler = IUpdateEventLoop::UpdateEvent::Handler([&](float dt) {
            if (m_dirtyViewport) {
                if (m_size.x > 0 && m_size.y > 0 && m_renderTarget) {
                    m_dirtyViewport = false;
                }
                m_viewportChanged.Signal();
            }
        });
        Interface<IUpdateEventLoop>::Get()->Subscribe(BroadcastEvent::PreUpdate, m_updateEventHandler);
    }

    cViewport::~cViewport() {
        m_disposeEvent.Signal();
    }

    void cViewport::SetWorld(cWorld* apWorld) {
        if (mpWorld != NULL && mpScene->WorldExists(mpWorld)) {
            mpWorld->SetIsSoundEmitter(false);
        }

        mpWorld = apWorld;
        if (mpWorld)
            mpWorld->SetIsSoundEmitter(true);

        mpRenderSettings->ResetVariables();
    }

    //-----------------------------------------------------------------------

    void cViewport::AddGuiSet(cGuiSet* apSet) {
        m_guiSets.push_back(apSet);
    }

    void cViewport::RemoveGuiSet(cGuiSet* apSet) {
        STLFindAndRemove(m_guiSets, apSet);
    }

    cGuiSetListIterator cViewport::GetGuiSetIterator() {
        return cGuiSetListIterator(&m_guiSets);
    }

    bool cViewport::IsValid() {
        if (m_size.x == 0 || m_size.y == 0) {
            return false;
        }
        return true;
    }

    void cViewport::bindToWindow(window::NativeWindowWrapper& window) {
        m_windowEventHandler = window::WindowEvent::Handler([&](window::WindowEventPayload& event) {
            switch (event.m_type) {
            case window::WindowEventType::ResizeWindowEvent:
                SetSize(cVector2l(event.payload.m_resizeWindow.m_width, event.payload.m_resizeWindow.m_height));
                break;
            default:
                break;
            }
        });

        window.ConnectWindowEventHandler(m_windowEventHandler);
        SetSize(window.GetWindowSize());
    }

    void cViewport::AddViewportCallback(iViewportCallback* apCallback) {
        mlstCallbacks.push_back(apCallback);
    }

    void cViewport::RemoveViewportCallback(iViewportCallback* apCallback) {
        STLFindAndRemove(mlstCallbacks, apCallback);
    }

    void cViewport::RunViewportCallbackMessage(eViewportMessage aMessage) {
        tViewportCallbackListIt it = mlstCallbacks.begin();
        for (; it != mlstCallbacks.begin(); ++it) {
            iViewportCallback* pCallback = *it;

            pCallback->RunMessage(aMessage);
        }
    }

    PrimaryViewport::PrimaryViewport() {

    }

    PrimaryViewport::PrimaryViewport(window::NativeWindowWrapper& window) {
        m_windowEventHandler = window::WindowEvent::Handler([&](window::WindowEventPayload& event) {
            switch (event.m_type) {
            case window::WindowEventType::ResizeWindowEvent:
                SetSize(cVector2l(event.payload.m_resizeWindow.m_width, event.payload.m_resizeWindow.m_height));
                break;
            default:
                break;
            }
        });

        m_updateEventHandler = IUpdateEventLoop::UpdateEvent::Handler([&](float dt) {
            if (m_dirtyViewport) {
                 if (m_size.x > 0 && m_size.y > 0) {
                     m_dirtyViewport = false;
                }
                m_viewportChanged.Signal();
            }
        });


        window.ConnectWindowEventHandler(m_windowEventHandler);
        SetSize(window.GetWindowSize());
        Interface<IUpdateEventLoop>::Get()->Subscribe(BroadcastEvent::PreUpdate, m_updateEventHandler);
    }

} // namespace hpl
