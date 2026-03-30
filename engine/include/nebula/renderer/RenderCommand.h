#pragma once
#include <glm/vec4.hpp>

namespace nebula::RenderCommand
{
    void setClearColor(const glm::vec4 &color);
    void clear();
}