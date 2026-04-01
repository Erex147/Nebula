#include "nebula/renderer/Tilemap.h"
#include <algorithm>

namespace nebula
{

    Tilemap::Tilemap(int mw, int mh, int tw, int th, int cols)
        : m_mapW(mw), m_mapH(mh), m_tileW(tw), m_tileH(th), m_atlasColumns(cols), m_tiles(mw * mh, -1) {}

    void Tilemap::setAtlas(std::shared_ptr<Texture> atlas) { m_atlas = atlas; }

    void Tilemap::setTile(int x, int y, int id)
    {
        if (x >= 0 && x < m_mapW && y >= 0 && y < m_mapH)
            m_tiles[y * m_mapW + x] = id;
    }

    int Tilemap::getTile(int x, int y) const
    {
        if (x < 0 || x >= m_mapW || y < 0 || y >= m_mapH)
            return -1;
        return m_tiles[y * m_mapW + x];
    }

    void Tilemap::draw(SpriteBatch &batch)
    {
        if (!m_atlas)
            return;

        if (m_atlasColumns <= 0)
            return;

        const float atlasTileW = (float)m_atlas->width() / (float)m_atlasColumns;
        const int atlasRows = std::max(1, (int)(m_atlas->height() / atlasTileW));
        const float atlasTileH = (float)m_atlas->height() / (float)atlasRows;

        float invW = 1.0f / (float)m_atlas->width();
        float invH = 1.0f / (float)m_atlas->height();
        const float insetU = 0.5f * invW;
        const float insetV = 0.5f * invH;

        for (int row = 0; row < m_mapH; row++)
        {
            for (int col = 0; col < m_mapW; col++)
            {
                int id = m_tiles[row * m_mapW + col];
                if (id < 0)
                    continue;

                float px = (float)(col * m_tileW);
                float py = (float)(row * m_tileH);

                // UV region for this tile id
                int tileCol = id % m_atlasColumns;
                int tileRow = id / m_atlasColumns;
                float u0 = (tileCol * atlasTileW) * invW + insetU;
                float v0 = (tileRow * atlasTileH) * invH + insetV;
                float u1 = ((tileCol + 1) * atlasTileW) * invW - insetU;
                float v1 = ((tileRow + 1) * atlasTileH) * invH - insetV;

                batch.drawRegion(*m_atlas,
                                 px, py, (float)m_tileW, (float)m_tileH,
                                 u0, v0, u1, v1);
            }
        }
    }

} // namespace nebula
