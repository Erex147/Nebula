#include "nebula/renderer/RenderCommand.h"
#include <glad/glad.h>

namespace nebula::RenderCommand
{

    void setClearColor(const glm::vec4 &c)
    {
        glClearColor(c.r, c.g, c.b, c.a);
    }

    void clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

}