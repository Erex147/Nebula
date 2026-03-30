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

glm::vec2 Camera2D::screenToWorld(const glm::vec2 &screenPos,
                                  const glm::vec2 &screenSize,
                                  bool originTopLeft) const
{
    if (screenSize.x == 0.0f || screenSize.y == 0.0f)
        return {0.0f, 0.0f};

    float x = (screenPos.x / screenSize.x) * 2.0f - 1.0f;
    float y = originTopLeft
                  ? 1.0f - (screenPos.y / screenSize.y) * 2.0f
                  : (screenPos.y / screenSize.y) * 2.0f - 1.0f;
    glm::vec4 world = m_invVP * glm::vec4(x, y, 0.0f, 1.0f);
    return {world.x / world.w, world.y / world.w};
}

glm::vec2 Camera2D::worldToScreen(const glm::vec2 &worldPos,
                                  const glm::vec2 &screenSize,
                                  bool originTopLeft) const
{
    glm::vec4 clip = m_vp * glm::vec4(worldPos, 0.0f, 1.0f);
    glm::vec2 ndc = {clip.x / clip.w, clip.y / clip.w};
    float x = ((ndc.x + 1.0f) * 0.5f) * screenSize.x;
    float y = originTopLeft
                  ? ((1.0f - ndc.y) * 0.5f) * screenSize.y
                  : ((ndc.y + 1.0f) * 0.5f) * screenSize.y;
    return {x, y};
}

void Camera2D::recalculate() {
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(m_position, 0.0f))
                * glm::scale   (glm::mat4(1.0f), glm::vec3(m_zoom, m_zoom, 1.0f));
    m_view = glm::inverse(t);
    m_vp   = m_projection * m_view;
    m_invVP = glm::inverse(m_vp);
}

} // namespace nebula
