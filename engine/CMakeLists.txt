add_library(nebula STATIC
    src/core/Window.cpp
    src/core/Application.cpp
    src/core/Input.cpp
    src/renderer/RenderCommand.cpp
    src/renderer/Shader.cpp
    src/renderer/Texture.cpp
    src/renderer/VertexArray.cpp
    src/renderer/SpriteBatch.cpp
    src/renderer/Tilemap.cpp
    src/renderer/FontRenderer.cpp
    src/renderer/TextureAtlas.cpp
    src/renderer/Animator.cpp
    src/math/Camera2D.cpp
    src/assets/AssetManager.cpp
    src/fx/ParticleSystem.cpp
    src/physics/Physics.cpp
    src/scene/SceneManager.cpp
    src/debug/DebugOverlay.cpp
    src/audio/AudioManager.cpp
    src/ui/UILayer.cpp
    src/gfx/Framebuffer.cpp
    src/gfx/LightRenderer.cpp
    src/gfx/PostProcessor.cpp
    ${CMAKE_SOURCE_DIR}/vendor/stb/stb_impl.cpp
    ${CMAKE_SOURCE_DIR}/vendor/miniaudio/miniaudio_impl.cpp
)

target_include_directories(nebula
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_SOURCE_DIR}/vendor/stb
    PRIVATE ${CMAKE_SOURCE_DIR}/vendor/miniaudio
)

target_link_libraries(nebula
    PUBLIC glfw
    PUBLIC glad
    PUBLIC glm::glm
    PUBLIC Freetype::Freetype
)
