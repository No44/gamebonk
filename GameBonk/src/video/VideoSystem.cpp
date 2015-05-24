#include <cstring>

#include "MMU.hpp"
#include "video/VideoSystem.hpp"

namespace GBonk
{
    namespace Video
    {

        VideoSystem::VideoSystem(uint8_t* m)
            : baseMem_(m),
            vram_(m + MMU::VIDEO_RAM),
            spriteAttrMem_(reinterpret_cast<ObjectAttribute*>(m + MMU::SPRITE_ATTRIB_MEMORY)),
            lcdc_(reinterpret_cast<LCDC*>(m + 0xFF40)),
            SCY_(m + 0xFF42),
            SCX_(m + 0xFF43),
            WY_(m + 0xFF4A),
            WX_(m + 0xFF4B),
            LY_(m + 0xFF44),
            LYC_(m + 0xFF45)
        {
            spritePatternTable_.setAddr(m, 0x8000);
            palettes_[0][0] = 0xFF0000FF;
            palettes_[0][1] = 0x00FF00FF;
            palettes_[0][2] = 0x0000FFFF;
            palettes_[0][3] = 0xFFFF00FF;

            palettes_[1][0] = 0xFFFFF0FF;
            palettes_[1][1] = 0xF0FF0FFF;
            palettes_[1][2] = 0x0F0F0FFF;
            palettes_[1][3] = 0xFF00FFFF;
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

        void VideoSystem::updateLCDC()
        {
            tilePatternTable_.setAddr(baseMem_, tile_pattern_table_addresses[lcdc_->tilePttrnAddrMode]);
            backgroundTable_.setAddr(baseMem_ + background_tile_table_addresses[lcdc_->bckgrdTileTableAddrMode]);
            windowTable_.setAddr(baseMem_ + window_tile_table_addresses[lcdc_->wdwTileTableAddrMode]);
        }

        void VideoSystem::render()
        {
            driver_.render();
        }

        static inline bool _skipsprite(int x, int y)
        {
          return x == 0 || x >= 160 + 8 || y == 0 || y >= 144+16;
        }

        void VideoSystem::drawLine()
        {
            drawLine(*LY_);
            *LY_ = (*LY_ + 1) % ScanLines;
        }

        void VideoSystem::drawAll()
        {
            for (int i = 0; i < ScreenHeight; ++i)
                drawLine(i);
        }

        void VideoSystem::drawLine(int line)
        {
            Sprite result(ScreenWidth, 1);
            result.x = 0;
            result.y = line;
          /*
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
            if (_skipsprite(line, attr.posx, attr.posy))
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
          */

          unsigned int spritesBackgroundCount = 0;
          if (lcdc_->backgroundDisplay)
          {
            if (line == 0)
              buildBackground_();
            
            int bckgrd_y = (*SCY_ + line) % ScreenHeight;
            int bckgrd_x = *SCX_;
            unsigned int bckgrd_pixy = bckgrd_y * fbwidth;
            for (unsigned int i = 0; i < ScreenWidth; ++i)
            {
                unsigned int pixel = backgroundMap_[bckgrd_pixy + ((bckgrd_x + i) % fbwidth)];
                if (pixel != 0x000000FF)
                    result.set(i, 0, pixel);
            }
          }
          driver_.draw(result);
        }

        void VideoSystem::buildBackground_()
        {
          for (int y = 0; y < TileRows; y++)
          {
            unsigned int line = y * fbwidth;
            for (int x=  0; x < TileCols; x++)
            {
              unsigned int tileId = backgroundTable_.tileId(x, y);
              Sprite s = tilePatternTable_.getSprite(tileId, palettes_[0]);
              std::memcpy(&backgroundMap_[line + x*TilePixSize], &s[0], s.width() * s.height());
            }
          }
        }

    }
}
