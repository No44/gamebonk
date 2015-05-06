#include "MMU.hpp"
#include "video/VideoSystem.hpp"

namespace GBonk
{
    namespace Video
    {

        VideoSystem::VideoSystem(uint8_t* v, uint8_t* s)
            : vram_(v),
            spriteAttrMem_(reinterpret_cast<ObjectAttribute*>(s))
        {
            spritePatternTable_.setAddr(v - MMU::VIDEO_RAM, 0x8000);
        }

        static const uint32_t background_tile_table_addresses[] = {
            0x9800,
            0x9C00
        };
        static const uint32_t window_tile_table_addresses[] = {
            0x9800,
            0x9C00
        };
        static const uint32_t tile_pattern_table_addresses[] = {
            0x8800,
            0x8000,
        };

        void VideoSystem::setLCDC(uint32_t val)
        {
            uint8_t* baseAddr = vram_ - MMU::VIDEO_RAM;
            
            lcdc_.value = val;
            tilePatternTable_.setAddr(baseAddr, tile_pattern_table_addresses[lcdc_.tilePttrnAddrMode]);
            backgroundTable_.setAddr(baseAddr + background_tile_table_addresses[lcdc_.bckgrdTileTableAddrMode]);
            windowTable_.setAddr(baseAddr + window_tile_table_addresses[lcdc_.wdwTileTableAddrMode]);
        }

        /*
        void VideoSystem::draw()
        {
            // std::vector<TilePattern> patterns;
            // At most 40 spr + background + window
            // patterns.reserve(40 + (32*32) * 2);
            // loop through OAM
            //  - priority sprites pushed front
            //  - no priority sprites pushed back
            // background and window in-between
            // display the vector

            if (lcdc_.backgroundDisplay)
            {
                uint32_t scrollx = this->scrollx_;
                for (int i = 0; i < tiles; ++i)
                {
                    uint32_t scrolly = this->scrolly_;
                    for (int j = 0; j < tiles; ++j)
                    {
                        unsigned int tile = backgroundTable_.tileId(i, j);
                        // tilePattern = tilePatternTable_.get(tile);
                        // draw tilePattern at (scrollx, scrolly)
                        scrolly = (scrolly + tileSize) % height;
                    }
                    scrollx = (scrollx + tileSize) % width;
                }
            }

            if (lcdc_.windowDisplay)
            {
                uint32_t wnpx = this->wndposx_;
                for (int i = 0; i < tiles && wnpx < width; ++i)
                {
                    uint32_t wnpy = this->wndposy_;
                    for (int j = 0; j < tiles && wnpy < height; ++j)
                    {
                        int tile = windowTable_.tileId(wnpx, wnpy);
                        // tilePattern = tilePatternTable_.get(tile);
                        // draw tilePattern at (scrollx, scrolly)
                        wnpy += tileSize;
                    }
                    wnpx += tileSize;
                }
            }
        }
        */

        static inline bool _skipsprite(int x, int y)
        {
          return x == 0 || x >= 160 + 8 || y == 0 || y >= 144+16;
        }

        void VideoSystem::draw()
        {
          if (!lcdc_.lcdOp)
            return;
          // total number of tiles/sprites:
          // - 40 sprites
          // - 32 * 32 tiles (background)
          // - 32 * 32 tiles (window)
          static const unsigned int TotalSprites = 40 + 32*32 * 2;
          std::vector<Sprite> sprites;
          sprites.resize(TotalSprites);

          // high prio is drawn on top of everything
          // low prio is drawn beneath bckground and window
          // hence, low prio sprites should appear first in the vector
          static const unsigned int HIGH_PRIO = 0;
          static const unsigned int LOW_PRIO = 1;
          unsigned int spritePrioCount[2] = {0, 0};
          unsigned int high_prio_idx = sprites.size() - 1;
          for (int i = 0; i < 40; ++i)
          {
            ObjectAttribute& attr = spriteAttrMem_[i];
            if (_skipsprite(attr.posx, attr.posy))
              continue;
            int index = attr.priority ? spritePrioCount[LOW_PRIO]++ : (spritePrioCount[HIGH_PRIO]++, high_prio_idx--);
            sprites[index] = spritePatternTable_.getSprite(attr.patternId, palettes_[attr.palette]);
            sprites[index].x = attr.posx - SPRITE_XPOS_ADJUST;
            sprites[index].y = attr.posy - SPRITE_YPOS_ADJUST;
            if (attr.xflip)
              sprites[index].flipx();
            if (attr.yflip)
              sprites[index].flipy();
            spritePrioCount[attr.priority]++;
          }

          unsigned int spritesBackgroundCount = 0;
          if (lcdc_.backgroundDisplay)
          {

          }
        }

    }
}
