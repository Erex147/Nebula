#pragma once
#include "nebula/ecs/Entity.h"
#include <vector>
#include <typeindex>

namespace nebula
{

    // type-erased base so World can store pools in a map
    class IComponentPool
    {
    public:
        virtual ~IComponentPool() = default;
        virtual void remove(EntityID e) = 0;
        virtual bool has(EntityID e) const = 0;
    };

    // sparse set: O(1) add/remove/has, cache-friendly dense iteration
    template <typename T>
    class ComponentPool : public IComponentPool
    {
    public:
        T &add(EntityID e, T component)
        {
            uint32_t idx = entityIndex(e);
            if (idx >= m_sparse.size())
                m_sparse.resize(idx + 1, EMPTY);

            m_sparse[idx] = (uint32_t)m_dense.size();
            m_dense.push_back(std::move(component));
            m_entities.push_back(e);
            return m_dense.back();
        }

        void remove(EntityID e) override
        {
            uint32_t idx = entityIndex(e);
            if (idx >= m_sparse.size() || m_sparse[idx] == EMPTY)
                return;

            uint32_t denseIdx = m_sparse[idx];
            uint32_t lastIdx = (uint32_t)m_dense.size() - 1;

            // swap-with-last to keep dense array packed
            if (denseIdx != lastIdx)
            {
                m_dense[denseIdx] = std::move(m_dense[lastIdx]);
                m_entities[denseIdx] = m_entities[lastIdx];
                m_sparse[entityIndex(m_entities[denseIdx])] = denseIdx;
            }

            m_dense.pop_back();
            m_entities.pop_back();
            m_sparse[idx] = EMPTY;
        }

        bool has(EntityID e) const override
        {
            uint32_t idx = entityIndex(e);
            return idx < m_sparse.size() && m_sparse[idx] != EMPTY;
        }

        T &get(EntityID e)
        {
            return m_dense[m_sparse[entityIndex(e)]];
        }

        const std::vector<EntityID> &entities() const { return m_entities; }
        size_t size() const { return m_dense.size(); }

    private:
        static constexpr uint32_t EMPTY = ~0u;
        std::vector<uint32_t> m_sparse;   // entity index  -> dense index
        std::vector<T> m_dense;           // tightly packed component data
        std::vector<EntityID> m_entities; // dense index -> entity (for reverse lookup)
    };

} // namespace nebula