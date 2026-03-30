#include "nebula/math/Camera2D.h"
#include <glm/gtc/matrix_transform.hpp>

namespace nebula {

Camera2D::Camera2D(float l, float r, float b, float t) {
    setProjection(l, r, b, t);
}

void Camera2D::setProjection(float l, float r, float b, float t) {
    m_projection = glm::ortho(l, r, b, t, -1.0f, 1.0f);
    recalculate();
}

void Camera2D::setPosition(const glm::vec2& pos) { m_position = pos; recalculate(); }
void Camera2D::move(const glm::vec2& d)           { m_position += d;  recalculate(); }
void Camera2D::setZoom(float z)                   { m_zoom = z;        recalculate(); }
void Camera2D::zoomBy(float f)                    { m_zoom *= f;       recalculate(); }

void Camera2D::recalculate() {
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(m_position, 0.0f))
                * glm::scale   (glm::mat4(1.0f), glm::vec3(m_zoom, m_zoom, 1.0f));
    m_view = glm::inverse(t);
    m_vp   = m_projection * m_view;
}

} // namespace nebula