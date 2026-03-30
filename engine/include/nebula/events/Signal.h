#pragma once
#include <functional>
#include <vector>
#include <cstdint>

namespace nebula
{

    using ListenerID = uint32_t;

    // Signal<T> — one emitter, many listeners
    // Use for object-level connections (e.g. button.onClick)
    template <typename... Args>
    class Signal
    {
    public:
        using Callback = std::function<void(Args...)>;

        ListenerID connect(Callback cb)
        {
            m_listeners.push_back({m_nextID++, std::move(cb)});
            return m_listeners.back().id;
        }

        void disconnect(ListenerID id)
        {
            m_listeners.erase(
                std::remove_if(m_listeners.begin(), m_listeners.end(),
                               [id](const Entry &e)
                               { return e.id == id; }),
                m_listeners.end());
        }

        void disconnectAll() { m_listeners.clear(); }

        void emit(Args... args) const
        {
            // copy in case a callback disconnects itself mid-emit
            auto copy = m_listeners;
            for (auto &e : copy)
                e.cb(args...);
        }

        size_t listenerCount() const { return m_listeners.size(); }

    private:
        struct Entry
        {
            ListenerID id;
            Callback cb;
        };
        std::vector<Entry> m_listeners;
        ListenerID m_nextID = 0;
    };

} // namespace nebula