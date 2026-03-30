#include "nebula/renderer/Texture.h"
#include <glad/glad.h>
#include <stdexcept>
#include <stb_image.h>

namespace nebula
{
    namespace
    {
        GLint toGLFilter(TextureFilter filter)
        {
            switch (filter)
            {
            case TextureFilter::Nearest:
                return GL_NEAREST;
            case TextureFilter::Linear:
                return GL_LINEAR;
            case TextureFilter::LinearMipmapLinear:
                return GL_LINEAR_MIPMAP_LINEAR;
            }
            return GL_LINEAR;
        }

        GLint toGLWrap(TextureWrap wrap)
        {
            switch (wrap)
            {
            case TextureWrap::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case TextureWrap::Repeat:
                return GL_REPEAT;
            case TextureWrap::MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            }
            return GL_CLAMP_TO_EDGE;
        }
    } // namespace

    TextureLoadOptions TextureLoadOptions::pixelArt(bool flip)
    {
        TextureLoadOptions options;
        options.flipVertically = flip;
        options.generateMipmaps = true;
        options.minFilter = TextureFilter::Nearest;
        options.magFilter = TextureFilter::Nearest;
        return options;
    }

    TextureLoadOptions TextureLoadOptions::smooth(bool flip)
    {
        TextureLoadOptions options;
        options.flipVertically = flip;
        options.generateMipmaps = true;
        options.minFilter = TextureFilter::LinearMipmapLinear;
        options.magFilter = TextureFilter::Linear;
        return options;
    }

    TextureLoadOptions TextureLoadOptions::renderTarget()
    {
        TextureLoadOptions options;
        options.flipVertically = false;
        options.generateMipmaps = false;
        options.minFilter = TextureFilter::Linear;
        options.magFilter = TextureFilter::Linear;
        return options;
    }

    Texture::Texture(const std::string &path, TextureLoadOptions options)
        : m_options(options)
    {
        stbi_set_flip_vertically_on_load(options.flipVertically ? 1 : 0);
        int channels;
        unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
        if (!data)
            throw std::runtime_error("Failed to load texture: " + path);
        upload(data, m_width, m_height, channels);
        stbi_image_free(data);
        stbi_set_flip_vertically_on_load(1);
    }

    Texture::Texture(int width, int height, TextureLoadOptions options)
        : m_width(width), m_height(height), m_options(options)
    {
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        applySampler();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture(uint32_t existingID, int w, int h, bool owned, TextureLoadOptions options)
        : m_id(existingID), m_width(w), m_height(h), m_owned(owned), m_options(options) {}

    std::unique_ptr<Texture> Texture::fromID(uint32_t id, int w, int h, TextureLoadOptions options)
    {
        return std::unique_ptr<Texture>(new Texture(id, w, h, false, options));
    }

    Texture::~Texture()
    {
        if (m_owned)
            glDeleteTextures(1, &m_id);
    }

    void Texture::bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

    void Texture::unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

    void Texture::upload(unsigned char *data, int w, int h, int channels)
    {
        GLenum internalFmt = (channels == 4) ? GL_RGBA8 : GL_RGB8;
        GLenum fmt = (channels == 4) ? GL_RGBA : GL_RGB;

        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, w, h, 0,
                     fmt, GL_UNSIGNED_BYTE, data);
        if (m_options.generateMipmaps)
            glGenerateMipmap(GL_TEXTURE_2D);
        applySampler();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::applySampler() const
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(m_options.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(m_options.wrapT));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(m_options.minFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(m_options.magFilter));
    }

} // namespace nebula
