#pragma once
#include <cstdint>

namespace nebula
{

    using EntityID = uint32_t;
    static constexpr EntityID NULL_ENTITY = ~0u;

    // lower 20 bits = slot index (up to ~1M live entities)
    // upper 12 bits = generation  (4096 reuses per slot)
    static constexpr uint32_t ENTITY_INDEX_BITS = 20;
    static constexpr uint32_t ENTITY_INDEX_MASK = (1u << ENTITY_INDEX_BITS) - 1u;

    inline uint32_t entityIndex(EntityID id) { return id & ENTITY_INDEX_MASK; }
    inline uint32_t entityGen(EntityID id) { return id >> ENTITY_INDEX_BITS; }
    inline EntityID makeEntity(uint32_t idx, uint32_t gen)
    {
        return idx | (gen << ENTITY_INDEX_BITS);
    }

} // namespace nebula