/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once


#include <cstdint>
#include <functional>
#include <queue>
#include <stack>
#include <mutex>
#include <thread>
#include <tuple>
#include <type_traits>

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>


namespace hpl
{
    //! A specialized type of EBus useful for eventing on a specific event type
    //! Whereas with EBus you would have to define a bus, implement the bus, and manage connecting and disconnecting from the bus
    //! Event only requires you declare an Event<>, and then connect Event<>::Handlers
    //! It is safe to connect or disconnect handlers during a signal, in this case we don't guarantee the signal will be dispatched to the disconnected handler
    //! Example Usage:
    //! @code{.cpp}
    //!      {
    //!          Event<int32_t> event; // An Event that can event a single int32_t
    //!          Event<int32_t>::Handler handler([](int32_t value) { DO_SOMETHING_WITH_VALUE(value); });
    //!          handler.Connect(event); // Our handler is now connected to the Event
    //!          event.Signal(1); // Our handlers lambda will now get invoked with the value 1
    //!      };
    //! @endcode

    template <typename... Params>
    class Event;

    template <typename... Params>
    class EventHandler;

    template <typename... Params>
    class QueuedEventLoopHandler;

    template <typename... Params>
    class Event final
    {
        friend class EventHandler<Params...>; // This is required in order to allow for relocating the handle pointers on a move

    public:

        using Callback = std::function<void(Params...)>;
        using Handler = EventHandler<Params...>;
        using QueuedEventHandler = QueuedEventLoopHandler<Params...>;

        Event() = default;
        Event(Event&& rhs);

        ~Event();

        Event& operator=(Event&& rhs);

        //! Take the handlers registered with the other event
        //! and move them to this event. The other will event
        //! will be cleared after call
        //! @param other event to move handlers
        Event& ClaimHandlers(Event&& other);

        //! Returns true if at least one handler is connected to this event.
        bool HasHandlerConnected() const;

        //! Disconnects all connected handlers.
        void DisconnectAllHandlers();

        //! Signal an event.
        //! signals are mutually exclusive, if a signal is already in progress, this call will block until the signal is complete
        //! @param params variadic set of event parameters
        void Signal(const Params&... params) const;

    private:

        //! Used internally to rebind all handlers from an old event to the current event instance
        void BindHandlerEventPointers();

        void Connect(Handler& handler) const;
        void Disconnect(Handler& handler) const;

    private:

        // Note that these are mutable because we want Signal() to be const, but we do a bunch of book-keeping during Signal()
        mutable std::vector<Handler*> m_handlers; //< Active handlers
        mutable std::vector<Handler*> m_addList; //< Handlers added during a Signal, pending being added to the active handlers
        mutable std::stack<size_t> m_freeList; //< Set of unused handler indices

        mutable bool m_updating = false; //< Raised during a Signal, false otherwise, used to guard m_handlers during handler iteration
        mutable std::recursive_mutex m_mutex; //< Mutex used to guard m_handlers during handler iteration
    };

    //! A handler class that can connect to an Event
    template <typename... Params>
    class EventHandler final
    {
        friend class Event<Params...>;

    public:
        using Callback = std::function<void(Params...)>;


        // We support default constructing of event handles (with no callback function being bound) to allow for better usage with container types
        // An unbound event handle cannot be added to an event and we do not support dynamically binding the callback post construction
        // (except for on assignment since that will also add the handle to the event; i.e. there is no way to unbind the callback after being added to an event)
        EventHandler() = default;
        explicit EventHandler(std::nullptr_t);
        explicit EventHandler(Callback callback);
        EventHandler(const EventHandler& rhs);
        EventHandler(EventHandler&& rhs);

        ~EventHandler();

        EventHandler& operator=(const EventHandler& rhs);
        EventHandler& operator=(EventHandler&& rhs);

        //! Connects the handler to the provided event.
        //! @param event the Event to connect to
        void Connect(Event<Params...>& event);

        //! Disconnects the handler from its connected event, does nothing if the event is not connected.
        void Disconnect();

        //! Returns true if this handler is connected to an event.
        //! @return boolean true if this handler is connected to an event
        bool IsConnected() const;

        //! Process queued
        void Process();

    private:

        //! Swaps the event handler pointers from the from instance to this instance
        //! @param from the handler instance we are replacing on the attached event
        void SwapEventHandlerPointers(const EventHandler& from);

        const Event<Params...>* m_event = nullptr; //< The connected event
        int32_t m_index = 0; //< Index into the add or handler vectors (negative means pending add)
        Callback m_callback; //< The lambda to invoke during events
    };

    template <typename... Params>
    EventHandler<Params...>::EventHandler(Callback callback)
        : m_callback(std::move(callback))
    {
    }

    template <typename... Params>
    EventHandler<Params...>::EventHandler(const EventHandler& rhs)
        : m_callback(rhs.m_callback)
        , m_event(rhs.m_event)
    {
        // Copy the callback and event, then perform a Connect to the event
        if (m_callback && m_event)
        {
            m_event->Connect(*this);
        }
        else
        {
            // It was not possible to connect to the event, set it to nullptr
            m_event = nullptr;
        }
    }

    template <typename... Params>
    EventHandler<Params...>::EventHandler(EventHandler&& rhs)
        : m_event(rhs.m_event)
        , m_index(rhs.m_index)
        , m_callback(std::move(rhs.m_callback))
    {
        // Moves all of the data of the r-value handle, fixup the event to point to them, and revert the r-value handle to it's original construction state
        rhs.m_event = nullptr;
        rhs.m_index = 0;

        SwapEventHandlerPointers(rhs);
    }


    template <typename... Params>
    EventHandler<Params...>::~EventHandler()
    {
        Disconnect();
    }

    template <typename... Params>
    EventHandler<Params...>& EventHandler<Params...>::operator=(const EventHandler& rhs)
    {
        // Copy the callback function, then perform a Connect with the new event
        if (this != &rhs)
        {
            Disconnect();
            m_callback = rhs.m_callback;
            m_event = rhs.m_event;
            // Copy the callback and event, then perform a Connect to the event
            if (m_callback && m_event)
            {
                m_event->Connect(*this);
            }
            else
            {
                // It was not possible to connect to the event, set it to nullptr
                m_event = nullptr;
            }
        }

        return *this;
    }


    template <typename... Params>
    EventHandler<Params...>& EventHandler<Params...>::operator=(EventHandler&& rhs)
    {
        if (this != &rhs)
        {
            Disconnect();
            // Moves all of the data of the r-value handle, fixup the event to point to them, and revert the r-value handle to it's original construction state
            m_event = rhs.m_event;
            m_index = rhs.m_index;
            m_callback = std::move(rhs.m_callback);

            rhs.m_event = nullptr;
            rhs.m_index = 0;

            SwapEventHandlerPointers(rhs);
        }

        return *this;
    }


    template <typename... Params>
    void EventHandler<Params...>::Connect(Event<Params...>& event)
    {
        // Cannot add an unbound event handle (no function callback) to an event, this is a programmer error
        // We explicitly do not support binding the callback after the handler has been constructed so we can just reject the event handle here
        ASSERT(m_callback &&  "Handler callback is null");
        if (!m_callback)
        {
            return;
        }

        ASSERT(!m_event &&  "Handler is already registered to an event, binding a handler to multiple events is unsupported");
        m_event = &event;

        event.Connect(*this);
    }


    template <typename... Params>
    void EventHandler<Params...>::Disconnect()
    {
        if (m_event)
        {
            m_event->Disconnect(*this);
        }
    }

    template <typename... Params>
    bool EventHandler<Params...>::IsConnected() const
    {
        return m_event != nullptr;
    }

    template <typename... Params>
    void EventHandler<Params...>::Process()
    {
        if (!m_callback)
        {
            return;
        }
    }

    template <typename... Params>
    void EventHandler<Params...>::SwapEventHandlerPointers([[maybe_unused]]const EventHandler& from)
    {
        // Find the pointer to the 'from' handler and point it to this handler
        if (m_event)
        {
            std::lock_guard<std::recursive_mutex> lk(m_event->m_mutex);
            // The index is negative if the handle is in the pending add list
            // The index can then be converted to the add list index in which it lives
            if (m_index < 0)
            {
                ASSERT(m_event->m_addList[-(m_index + 1)] == &from &&  "From handle does not match");
                m_event->m_addList[-(m_index + 1)] = this;
            }
            else
            {
                ASSERT(m_event->m_handlers[m_index] == &from &&  "From handle does not match");
                m_event->m_handlers[m_index] = this;
            }
        }
    }


    template <typename... Params>
    Event<Params...>::Event(Event&& rhs)
        : m_handlers(std::move(rhs.m_handlers))
        , m_addList(std::move(rhs.m_addList))
        , m_freeList(std::move(rhs.m_freeList))
        , m_updating(rhs.m_updating)
    {
        // Move all sub-objects into this event and fixup each handle to point to this event
        // Revert the r-value event to it's default state (the moves should do it but PODs need to be set)
        BindHandlerEventPointers();
        rhs.m_updating = false;
    }


    template <typename... Params>
    Event<Params...>::~Event()
    {
        DisconnectAllHandlers();
    }


    template <typename... Params>
    Event<Params...>& Event<Params...>::operator=(Event&& rhs)
    {
        // Remove all previous handles which will update them as needed
        // Move all sub-objects into this event and fixup each handle to point to this event
        // Revert the r-value event to it's default state (the moves should do it but PODs need to be set)
        DisconnectAllHandlers();

        m_handlers = std::move(rhs.m_handlers);
        m_addList = std::move(rhs.m_addList);
        m_freeList = std::move(rhs.m_freeList);
        m_updating = rhs.m_updating;

        BindHandlerEventPointers();

        rhs.m_updating = false;

        return *this;
    }


    template <typename... Params>
    auto Event<Params...>::ClaimHandlers(Event&& other) -> Event&
    {
        auto handlers = std::move(other.m_handlers);
        auto addList = std::move(other.m_addList);
        other.m_freeList = {};
        other.m_updating = false;

        std::array handlerContainers{ &handlers, &addList };
        for (std::vector<Handler*>* handlerList : handlerContainers)
        {
            for (Handler* handler : *handlerList)
            {
                if (handler != nullptr)
                {
                    handler->m_index = 0;
                    handler->m_event = this;
                    Connect(*handler);
                }
            }
        }

        return *this;
    }


    template <typename... Params>
    bool Event<Params...>::HasHandlerConnected() const
    {
        for (Handler* handler : m_handlers)
        {
            if (handler)
            {
                return true;
            }
        }

        return false;
    }


    template <typename... Params>
    void Event<Params...>::DisconnectAllHandlers()
    {
        // Clear all our added handlers
        for (Handler* handler : m_handlers)
        {
            if (handler)
            {
                ASSERT(handler->m_event == this && "Entry event does not match");
                handler->Disconnect();
            }
        }

        // Clear any handlers still pending registration
        for (Handler* handler : m_addList)
        {
            ASSERT(handler && "NULL handler encountered in Event addList");
            ASSERT(handler->m_event == this && "Entry event does not match");
            handler->Disconnect();
        }

        // Free up any owned memory
        std::vector<Handler*> freeHandlers;
        m_handlers.swap(freeHandlers);
        std::vector<Handler*> freeAdds;
        m_addList.swap(freeAdds);
        std::stack<size_t> freeFree;
        m_freeList.swap(freeFree);
    }


    template <typename... Params>
    void Event<Params...>::Signal(const Params&... params) const
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        m_updating = true;

        // Trigger all added handler callbacks
        for (Handler* handler : m_handlers)
        {
            if (handler)
            {
                handler->m_callback(params...);
            }
        }

        // Update our handlers if we have pending adds
        if (!m_addList.empty())
        {
            for (Handler* handler : m_addList)
            {
                ASSERT(handler &&  "NULL handler encountered in Event addList");
                if (m_freeList.empty())
                {
                    handler->m_index = static_cast<int32_t>(m_handlers.size());
                    m_handlers.push_back(handler);
                }
                else
                {
                    handler->m_index = static_cast<int32_t>(m_freeList.top());
                    m_freeList.pop();

                    ASSERT(m_handlers[handler->m_index] == nullptr &&  "Callback already registered");
                    m_handlers[handler->m_index] = handler;
                }
            }
            m_addList.clear();
        }

        m_updating = false;
    }


    template <typename... Params>
    inline void Event<Params...>::BindHandlerEventPointers()
    {
        for (Handler* handler : m_handlers)
        {
            if (handler)
            {
                // This should have happened as part of a move so none of the pointers should refer to this event (they should also all refer to the same event)
                ASSERT(handler->m_event != this &&  "Should not refer to this");
                handler->m_event = this;
            }
        }

        for (Handler* handler : m_addList)
        {
            // This should have happened as part of a move so none of the pointers should refer to this event (they should also all refer to the same event)
            ASSERT(handler &&  "NULL handler encountered in Event addList");
            ASSERT(handler->m_event != this &&  "Should not refer to this");
            handler->m_event = this;
        }
    }


    template <typename... Params>
    inline void Event<Params...>::Connect(Handler& handler) const
    {
        std::lock_guard<std::recursive_mutex> lk(m_mutex);
        if (m_updating)
        {
            handler.m_index = -static_cast<int32_t>(m_addList.size() + 1);
            m_addList.push_back(&handler);
            return;
        }

        if (m_freeList.empty())
        {
            handler.m_index = static_cast<int32_t>(m_handlers.size());
            m_handlers.push_back(&handler);
        }
        else
        {
            handler.m_index = static_cast<int32_t>(m_freeList.top());
            m_freeList.pop();

            ASSERT(m_handlers[handler.m_index] == nullptr &&  "Replacing non nullptr event");
            m_handlers[handler.m_index] = &handler;
        }
    }


    template <typename... Params>
    inline void Event<Params...>::Disconnect(Handler& eventHandle) const
    {
        std::lock_guard<std::recursive_mutex> lk(m_mutex);
        ASSERT(eventHandle.m_event == this &&  "Trying to remove a handler bound to a different event");

        int32_t index = eventHandle.m_index;
        if (index < 0)
        {
            ASSERT(m_addList[-(index + 1)] == &eventHandle &&  "Entry does not refer to handle");
            m_addList[-(index + 1)] = nullptr;
        }
        else
        {
            ASSERT(m_handlers[index] == &eventHandle &&  "Entry does not refer to handle");
            m_handlers[index] = nullptr;
            m_freeList.push(index);
        }

        eventHandle.m_event = nullptr;
    }

}

