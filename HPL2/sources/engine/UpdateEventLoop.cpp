#include "math/Crc32.h"
#include <engine/UpdateEventLoop.h>
#include <string_view>

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

namespace hpl {

    UpdateEventLoop::UpdateEventLoop() {
    }

    UpdateEventLoop::~UpdateEventLoop() {
    }

    void UpdateEventLoop::ChangeEventGroup(std::string_view name) {
        math::Crc32 newId(name);
        if (m_activeEventGroup == newId) {
            return;
        }
        for (auto& group : m_eventGroups) {
            if (group.m_id == m_activeEventGroup) {
                group.m_broadcast->m_leaveEventGroup.Signal();
                break;
            }
        }

        for (auto& group : m_eventGroups) {
            if (group.m_id == newId) {
                m_activeEventGroup = newId;
                group.m_broadcast->m_enterEventGroup.Signal();
                break;
            }
        }
    }

    void UpdateEventLoop::CreateEventGroup(std::string_view name) {
        math::Crc32 id(name);
        auto& group = m_eventGroups.emplace_back();
        group.m_name = name;
        group.m_id = id;
    }

    void UpdateEventLoop::DestroyEventGroup(std::string_view name) {
        math::Crc32 id(name);
        for (auto it = m_eventGroups.begin(); it != m_eventGroups.end(); ++it) {
            if (it->m_id == id) {
                m_eventGroups.erase(it);
                return;
            }
        }
    }

    void UpdateEventLoop::Broadcast(BroadcastEvent event, float value) {
        ASSERT(static_cast<size_t>(event) < m_events.size() && "Event out of range");
        m_events[static_cast<size_t>(event)].Signal(value);
        for (auto& group : m_eventGroups) {
            if (group.m_id == m_activeEventGroup) {
                group.m_broadcast->m_events[static_cast<size_t>(event)].Signal(value);
                break;
            }
        }
    }
    void UpdateEventLoop::BroadcastToAll(BroadcastEvent event, float value) {
        ASSERT(static_cast<size_t>(event) < m_events.size() && "Event out of range");
        m_events[static_cast<size_t>(event)].Signal(value);
        for (auto& group : m_eventGroups) {
            group.m_broadcast->m_events[static_cast<size_t>(event)].Signal(value);
        }
    }

    const std::string_view UpdateEventLoop::GetActiveEventGroup() const {
        for (auto& group : m_eventGroups) {
            if (group.m_id == m_activeEventGroup) {
                return group.m_name;
            }
        }
        return std::string_view();
    }

    void UpdateEventLoop::Subscribe(BroadcastEvent event, IUpdateEventLoop::UpdateEvent::Handler& handler) {
        handler.Connect(m_events[static_cast<size_t>(event)]);
    }

    void UpdateEventLoop::Subscribe(
        const std::string_view groupName, BroadcastEvent event, IUpdateEventLoop::UpdateEvent::Handler& handler) {
        ASSERT(static_cast<size_t>(event) < m_events.size() && "Event out of range");
        math::Crc32 crc(groupName);
        for (auto& group : m_eventGroups) {
            if (group.m_id == groupName) {
                handler.Connect(group.m_broadcast->m_events[static_cast<size_t>(event)]);
                return;
            }
        }
    }
} // namespace hpl
