
#pragma once

// Core Event Loop
#include "engine/Event.h"
#include <engine/IUpdateEventLoop.h>
#include <engine/RTTI.h>
#include <memory>
#include <string>
#include <string_view>

#include <math/Crc32.h>
#include <vector>

namespace hpl {

    class UpdateEventLoop final : public IUpdateEventLoop {
        HPL_RTTI_IMPL_CLASS(IUpdateEventLoop, UpdateEventLoop, "{eb97278b-a219-4051-b3b5-d30f95230d46}")
    public:
        //store event handlers separately for each event group

        struct EventGroupBroadcast {
            ChangeGroupEvent m_enterEventGroup;
            ChangeGroupEvent m_leaveEventGroup;
            std::array<UpdateEvent, static_cast<size_t>(BroadcastEvent::LastEnum)> m_events;
        };

        struct EventGroup {
            std::string m_name = "";
            math::Crc32 m_id = math::Crc32();
            std::unique_ptr<EventGroupBroadcast> m_broadcast = std::make_unique<EventGroupBroadcast>();
        };

        UpdateEventLoop();
        ~UpdateEventLoop();

        virtual void ChangeEventGroup(std::string_view name) override;

        virtual const std::string_view GetActiveEventGroup() const override;

        virtual void CreateEventGroup(std::string_view name) override;
        virtual void DestroyEventGroup(std::string_view name) override;

        // broadcast to the global event group and the active event group
        virtual void Broadcast(BroadcastEvent event, float value = 0) override;
        // broadcast to all event groups
        virtual void BroadcastToAll(BroadcastEvent event, float value = 0) override;
        
        virtual void Subscribe(BroadcastEvent event, UpdateEvent::Handler& handler) override;
        virtual void Subscribe(const std::string_view id, BroadcastEvent event, UpdateEvent::Handler& handler) override;

    private:
        std::vector<EventGroup> m_eventGroups;
        math::Crc32 m_activeEventGroup = math::Crc32();
        std::array<UpdateEvent, static_cast<size_t>(BroadcastEvent::LastEnum)> m_events;
    };
} // namespace hpl