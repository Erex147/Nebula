#include "nebula/renderer/VertexArray.h"
#include <glad/glad.h>

namespace nebula {

static uint32_t typeSize(uint32_t type) {
    switch (type) {
        case GL_FLOAT:        return 4;
        case GL_UNSIGNED_INT: return 4;
        case GL_UNSIGNED_BYTE:return 1;
    }
    return 0;
}

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void VertexArray::bind()   const { glBindVertexArray(m_vao); }
void VertexArray::unbind() const { glBindVertexArray(0); }

void VertexArray::setVertexData(const void* data, size_t size, bool dynamic) {
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data,
                 dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

void VertexArray::setIndexData(const uint32_t* indices, uint32_t count) {
    m_indexCount = count;
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

void VertexArray::setLayout(const std::vector<BufferElement>& elements) {
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    uint32_t stride = 0;
    for (auto& e : elements)
        stride += e.count * typeSize(e.type);

    uint64_t offset = 0;
    for (uint32_t i = 0; i < elements.size(); i++) {
        auto& e = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, e.count, e.type,
                              e.normalized ? GL_TRUE : GL_FALSE,
                              stride, (const void*)offset);
        offset += e.count * typeSize(e.type);
    }
}

} // namespace nebula