#pragma once
#include <glm/glm.hpp>

namespace nebula {

class Camera2D {
public:
    Camera2D(float left, float right, float bottom, float top);

    void setProjection(float left, float right, float bottom, float top);
    void setPosition(const glm::vec2& pos);
    void move(const glm::vec2& delta);
    void setZoom(float zoom);
    void zoomBy(float factor);

    const glm::mat4& viewProjection() const { return m_vp; }
    const glm::vec2& position()       const { return m_position; }
    float            zoomLevel()      const { return m_zoom; }

private:
    glm::mat4 m_projection = glm::mat4(1.0f);
    glm::mat4 m_view       = glm::mat4(1.0f);
    glm::mat4 m_vp         = glm::mat4(1.0f);
    glm::vec2 m_position   = {0.0f, 0.0f};
    float     m_zoom       = 1.0f;

    void recalculate();
};

} // namespace nebula