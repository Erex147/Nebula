#pragma once
#include <vector>
#include <cstdint>

namespace nebula {

struct BufferElement {
    uint32_t type;       // GL_FLOAT, GL_UNSIGNED_BYTE, etc.
    uint32_t count;      // number of components (e.g. 2 for vec2)
    bool     normalized;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void bind()   const;
    void unbind() const;

    void setVertexData(const void* data, size_t size, bool dynamic = true);
    void setIndexData (const uint32_t* indices, uint32_t count);
    void setLayout    (const std::vector<BufferElement>& elements);

    uint32_t indexCount() const { return m_indexCount; }

private:
    uint32_t m_vao        = 0;
    uint32_t m_vbo        = 0;
    uint32_t m_ebo        = 0;
    uint32_t m_indexCount = 0;
};

} // namespace nebula