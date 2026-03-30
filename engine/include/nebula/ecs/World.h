#pragma once
#include "nebula/ecs/Entity.h"
#include "nebula/ecs/ComponentPool.h"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <cassert>

namespace nebula
{

    // forward declare so View can reference World
    class World;

    // ---- View ----------------------------------------------------------------
    // Iterates the first requested component's pool.
    // Skips any entity that is missing one of the other requested components.
    template <typename... Cs>
    class View
    {
    public:
        class Iterator
        {
        public:
            Iterator(World &world, const std::vector<EntityID> &entities, size_t idx)
                : m_world(world), m_entities(entities), m_idx(idx) { skip(); }

            EntityID operator*() const { return m_entities[m_idx]; }
            Iterator &operator++()
            {
                ++m_idx;
                skip();
                return *this;
            }
            bool operator!=(const Iterator &o) const { return m_idx != o.m_idx; }

        private:
            void skip(); // defined after World
            World &m_world;
            const std::vector<EntityID> &m_entities;
            size_t m_idx;
        };

        View(World &world, const std::vector<EntityID> &entities)
            : m_world(world), m_entities(entities) {}

        Iterator begin() { return Iterator(m_world, m_entities, 0); }
        Iterator end() { return Iterator(m_world, m_entities, m_entities.size()); }

        // inside View class, replace the each definition with just a declaration
        template <typename Func>
        void each(Func &&fn);

    private:
        World &m_world;
        const std::vector<EntityID> &m_entities;
    };

    // ---- World ---------------------------------------------------------------
    class World
    {
    public:
        // ---- entity lifecycle ----

        EntityID create()
        {
            uint32_t idx;
            if (!m_free.empty())
            {
                idx = m_free.back();
                m_free.pop_back();
            }
            else
            {
                idx = (uint32_t)m_generations.size();
                m_generations.push_back(0);
            }
            return makeEntity(idx, m_generations[idx]);
        }

        void destroy(EntityID e)
        {
            if (!alive(e))
                return;
            for (auto &[_, pool] : m_pools)
                pool->remove(e);
            uint32_t idx = entityIndex(e);
            m_generations[idx]++;
            m_free.push_back(idx);
        }

        bool alive(EntityID e) const
        {
            uint32_t idx = entityIndex(e);
            return idx < m_generations.size() && m_generations[idx] == entityGen(e);
        }

        // ---- component operations ----

        template <typename T>
        T &add(EntityID e, T component = T{})
        {
            assert(alive(e));
            return pool<T>().add(e, std::move(component));
        }

        template <typename T>
        void remove(EntityID e) { pool<T>().remove(e); }

        template <typename T>
        T &get(EntityID e)
        {
            assert(has<T>(e));
            return pool<T>().get(e);
        }

        template <typename T>
        bool has(EntityID e) const
        {
            auto it = m_pools.find(std::type_index(typeid(T)));
            if (it == m_pools.end())
                return false;
            return it->second->has(e);
        }

        // ---- views ----

        // Iterates entities that have ALL of Cs...
        // Uses the first type's pool as the iteration source.
        template <typename First, typename... Rest>
        View<First, Rest...> view()
        {
            return View<First, Rest...>(*this, pool<First>().entities());
        }

        size_t entityCount() const { return m_generations.size() - m_free.size(); }

    private:
        template <typename T>
        ComponentPool<T> &pool()
        {
            auto key = std::type_index(typeid(T));
            auto it = m_pools.find(key);
            if (it == m_pools.end())
            {
                auto [ins, _] = m_pools.emplace(key, std::make_unique<ComponentPool<T>>());
                return static_cast<ComponentPool<T> &>(*ins->second);
            }
            return static_cast<ComponentPool<T> &>(*it->second);
        }

        std::vector<uint32_t> m_generations;
        std::vector<uint32_t> m_free;
        std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> m_pools;
    };

    // ---- View::Iterator::skip — defined here because it needs World ----
    template <typename... Cs>
    void View<Cs...>::Iterator::skip()
    {
        while (m_idx < m_entities.size())
        {
            EntityID e = m_entities[m_idx];
            if ((m_world.has<Cs>(e) && ...))
                break;
            ++m_idx;
        }
    }

    // ---- View::each — defined here because it needs complete World ----
    template <typename... Cs>
    template <typename Func>
    void View<Cs...>::each(Func &&fn)
    {
        for (EntityID e : *this)
            fn(e, m_world.get<Cs>(e)...);
    }

} // namespace nebula