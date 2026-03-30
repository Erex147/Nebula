#pragma once
#include <string>
#include <cstdint>

namespace nebula {

class Texture {
public:
    explicit Texture(const std::string& path);
    Texture(int width, int height); // blank texture
    ~Texture();

    void bind(uint32_t slot = 0) const;
    void unbind() const;

    int      width()  const { return m_width; }
    int      height() const { return m_height; }
    uint32_t id()     const { return m_id; }

private:
    uint32_t m_id     = 0;
    int      m_width  = 0;
    int      m_height = 0;

    void upload(unsigned char* data, int w, int h, int channels);
};

} // namespace nebula