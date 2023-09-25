#pragma once

#include "engine/RTTI.h"
#include <engine/Event.h>
#include <math/Crc32.h>

namespace hpl {

    enum class BroadcastEvent {
        OnPostBufferSwap,
        OnStart,
        OnDraw,
        OnPostRender,
        PreUpdate,
        Update,
        PostUpdate,
        OnQuit,
        OnExit,
        Reset,
        OnPauseUpdate,

        AppGotInputFocus,
        AppGotMouseFocus,
        AppGotVisibility,

        AppLostInputFocus,
        AppLostMouseFocus,
        AppLostVisibility,

        AppDeviceWasPlugged,
        AppDeviceWasRemoved,

        LastEnum
    };

    class IUpdateEventLoop {
        HPL_RTTI_CLASS(IUpdateEventLoop, "{cb713dfb-d6d9-4f41-9021-80af7827f2f1}")
    public:
        using UpdateEvent = hpl::Event<float>;
        using ChangeGroupEvent = hpl::Event<>;

        virtual void ChangeEventGroup(std::string_view name) = 0;

        virtual void CreateEventGroup(std::string_view name) = 0;
        virtual void DestroyEventGroup(std::string_view name) = 0;

        virtual const std::string_view GetActiveEventGroup() const = 0;
        
        virtual void Broadcast(BroadcastEvent event, float value = 0) = 0;
        virtual void BroadcastToAll(BroadcastEvent event, float value = 0) = 0;

        virtual void Subscribe(BroadcastEvent event, UpdateEvent::Handler& handler) = 0;
        virtual void Subscribe(const std::string_view group, BroadcastEvent event, UpdateEvent::Handler& handler) = 0;
    private:
    };

} // namespace hpl