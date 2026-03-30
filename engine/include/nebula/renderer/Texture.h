#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace nebula
{
    enum class TextureFilter
    {
        Nearest,
        Linear,
        LinearMipmapLinear
    };

    enum class TextureWrap
    {
        ClampToEdge,
        Repeat,
        MirroredRepeat
    };

    struct TextureLoadOptions
    {
        bool flipVertically = true;
        bool generateMipmaps = true;
        TextureFilter minFilter = TextureFilter::LinearMipmapLinear;
        TextureFilter magFilter = TextureFilter::Nearest;
        TextureWrap wrapS = TextureWrap::ClampToEdge;
        TextureWrap wrapT = TextureWrap::ClampToEdge;

        static TextureLoadOptions pixelArt(bool flipVertically = true);
        static TextureLoadOptions smooth(bool flipVertically = true);
        static TextureLoadOptions renderTarget();

        bool operator==(const TextureLoadOptions &other) const
        {
            return flipVertically == other.flipVertically &&
                   generateMipmaps == other.generateMipmaps &&
                   minFilter == other.minFilter &&
                   magFilter == other.magFilter &&
                   wrapS == other.wrapS &&
                   wrapT == other.wrapT;
        }
    };

    class Texture
    {
    public:
        explicit Texture(const std::string &path,
                         TextureLoadOptions options = TextureLoadOptions{});
        Texture(int width, int height,
                TextureLoadOptions options = TextureLoadOptions::renderTarget());
        ~Texture();

        static std::unique_ptr<Texture> fromID(uint32_t id, int w, int h,
                                               TextureLoadOptions options = TextureLoadOptions::renderTarget());

        void bind(uint32_t slot = 0) const;
        void unbind() const;

        int width() const { return m_width; }
        int height() const { return m_height; }
        uint32_t id() const { return m_id; }
        const TextureLoadOptions &options() const { return m_options; }

    private:
        Texture(uint32_t existingID, int w, int h, bool owned, TextureLoadOptions options);

        uint32_t m_id = 0;
        int m_width = 0;
        int m_height = 0;
        bool m_owned = true;
        TextureLoadOptions m_options;

        void upload(unsigned char *data, int w, int h, int channels);
        void applySampler() const;
    };

} // namespace nebula
