#pragma once
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>
#include <cstdint>

namespace nebula
{

    using ListenerID = uint32_t;

    class EventBus
    {
    public:
        template <typename T>
        static ListenerID on(std::function<void(const T &)> cb)
        {
            auto &b = getBucket<T>();
            ListenerID id = s_nextID++;
            b.listeners.push_back({id, [cb](const void *e)
                                   {
                                       cb(*static_cast<const T *>(e));
                                   }});
            return id;
        }

        template <typename T, typename Obj>
        static ListenerID on(Obj *obj, void (Obj::*method)(const T &))
        {
            return on<T>([obj, method](const T &e)
                         { (obj->*method)(e); });
        }

        template <typename T>
        static void off(ListenerID id)
        {
            auto &b = getBucket<T>();
            b.listeners.erase(
                std::remove_if(b.listeners.begin(), b.listeners.end(),
                               [id](const Entry &e)
                               { return e.id == id; }),
                b.listeners.end());
        }

        template <typename T>
        static void emit(const T &event)
        {
            auto it = s_buckets.find(std::type_index(typeid(T)));
            if (it == s_buckets.end())
                return;
            auto copy = it->second->listeners;
            for (auto &e : copy)
                e.cb(&event);
        }

        template <typename T>
        static void defer(T event)
        {
            s_deferred.push_back([ev = std::move(event)]()
                                 { EventBus::emit(ev); });
        }

        static void flushDeferred()
        {
            auto pending = std::move(s_deferred);
            s_deferred.clear();
            for (auto &fn : pending)
                fn();
        }

        static void clearAll()
        {
            s_buckets.clear();
            s_deferred.clear();
        }

    private:
        struct Entry
        {
            ListenerID id;
            std::function<void(const void *)> cb;
        };
        struct Bucket
        {
            std::vector<Entry> listeners;
        };

        template <typename T>
        static Bucket &getBucket()
        {
            auto key = std::type_index(typeid(T));
            auto it = s_buckets.find(key);
            if (it == s_buckets.end())
            {
                auto [ins, _] = s_buckets.emplace(
                    key, std::make_unique<Bucket>());
                return *ins->second;
            }
            return *it->second;
        }

        static inline std::unordered_map<std::type_index,
                                         std::unique_ptr<Bucket>>
            s_buckets;
        static inline std::vector<std::function<void()>> s_deferred;
        static inline ListenerID s_nextID = 0;
    };

} // namespace nebula