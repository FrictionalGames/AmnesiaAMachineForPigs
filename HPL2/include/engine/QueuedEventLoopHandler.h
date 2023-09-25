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

#include "engine/Event.h"
#include "engine/IUpdateEventLoop.h"
#include "engine/Interface.h"
#include "engine/UpdateEventLoop.h"

namespace hpl {

/**
* Events are queued and processed on the event loop.
*/
template<typename... Params>
class QueuedEventLoopHandler
{
public:
    using TargetEvent = hpl::Event<Params...>;

    struct QueueEventOptions {
    public:

        inline QueueEventOptions()
            : onBegin([]() {
            })
            , onEnd([]() {

            })
            , filter([](Params...) {
                return true;
            }) {
        }
        inline QueueEventOptions(
            std::function<void()> begin, std::function<void()> end, std::function<bool(Params...)> filter

            )
            : onBegin(begin)
            , onEnd(end)
            , filter(filter) {

        }
        /**
        * called before processing queued events
        */
        std::function<void()> onBegin;
        /**
        * called after all queued events have been processed
        */
        std::function<void()> onEnd;
        /**
        * a filter function that returns true if the event should be queued
        */
        std::function<bool(Params...)> filter;
    };
    inline QueuedEventLoopHandler(
        BroadcastEvent event, typename TargetEvent::Callback callback, const QueueEventOptions options = QueueEventOptions{})
        :
        m_dispatchHandler([&, options, callback](float value) {
            std::lock_guard<std::mutex> lock(m_mutex);
            options.onBegin();
            while(!m_queuedEvents.empty()) {
                auto& data = m_queuedEvents.front();
                std::apply(callback, data);
                m_queuedEvents.pop();
            }
            options.onEnd();
        }),
        m_handler([&, options](Params... params) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if(options.filter(params...)) {
                m_queuedEvents.emplace(std::tuple<Params...>(params...));
            }
        }), m_broadcastEvent(event)
    {
    }

    inline void Connect(TargetEvent& event) {
        Interface<IUpdateEventLoop>::Get()->Subscribe(m_broadcastEvent, m_dispatchHandler);
        m_handler.Connect(event);
    }

    QueuedEventLoopHandler(const QueuedEventLoopHandler&) = delete;
    QueuedEventLoopHandler(QueuedEventLoopHandler&&) = delete;
    QueuedEventLoopHandler& operator=(const QueuedEventLoopHandler&) = delete;
    QueuedEventLoopHandler& operator=(QueuedEventLoopHandler&&) = delete;
private:
    hpl::IUpdateEventLoop::UpdateEvent::Handler m_dispatchHandler;
    typename hpl::Event<Params...>::Handler m_handler;
    std::queue<std::tuple<typename std::remove_reference<Params>::type...>> m_queuedEvents;
    std::mutex m_mutex;
    BroadcastEvent m_broadcastEvent;
};

} // namespace hpl
