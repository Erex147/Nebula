#pragma once
#include <vector>
#include <memory>
#include "nebula/renderer/Texture.h"
#include "nebula/renderer/SpriteBatch.h"

namespace nebula
{

    class Tilemap
    {
    public:
        // atlasColumns = how many tiles per row in your atlas texture
        Tilemap(int mapW, int mapH, int tileW, int tileH, int atlasColumns = 1);

        void setAtlas(std::shared_ptr<Texture> atlas);
        void setTile(int x, int y, int tileId); // -1 = empty
        int getTile(int x, int y) const;

        void draw(SpriteBatch &batch);

        int mapWidth() const { return m_mapW; }
        int mapHeight() const { return m_mapH; }
        int tileWidth() const { return m_tileW; }
        int tileHeight() const { return m_tileH; }

    private:
        int m_mapW, m_mapH;
        int m_tileW, m_tileH;
        int m_atlasColumns;
        std::vector<int> m_tiles;
        std::shared_ptr<Texture> m_atlas;
    };

} // namespace nebula