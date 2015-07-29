#include <cstring>

#include "MMU.hpp"
#include "utility.hpp"
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
            backgroundMap_.resize(256 * 256);
            windowMap_.resize(256 * 256);
            sprites_.resize(SpriteCount);
            
            for (auto& s : sprites_)
                s.x = ScreenWidth, s.y = ScreenHeight;
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
            driver_.pumpEvents();
        }

        void VideoSystem::forcePumpEvents()
        {
            driver_.pumpEvents();
        }

        void VideoSystem::onVramWrite(unsigned int value, unsigned int addr)
        {
            // todo: if writing in pattern table, need to update sprites
        }

        void VideoSystem::onVramWritew(unsigned int value, unsigned int addr)
        {
        }

        void VideoSystem::onOAMWrite(unsigned int value, unsigned int oldValue, unsigned int addr)
        {
            return; // tmp: debug release

            // get previous multiple of 4: that's our sprite index
            static const unsigned int SpriteAddrMask = 3;
            unsigned int oamBaseAddr = addr & ~SpriteAddrMask;
            unsigned int spriteIdx = (oamBaseAddr - MMU::SPRITE_ATTRIB_MEMORY) / sizeof(ObjectAttribute);
            Sprite& s = sprites_[spriteIdx];
            Sprite::SizeMode mode = static_cast<Sprite::SizeMode>(lcdc_->spriteSizeMode);
            ObjectAttribute& attr = spriteAttrMem_[spriteIdx];

            switch (addr & SpriteAddrMask)
            {
            case 0: {
                // posy
                unsigned int newPosY = value;
                unsigned int oldY = s.y;

                // possible opti sur les lignes qui overlappent ?
                for (unsigned int end = std::min(ScreenHeight, oldY + Sprite::ModeHeight[mode]); oldY < end; ++oldY)
                    spriteLines_[oldY].remove(&s);
                for (unsigned int end = std::min(ScreenHeight, value + Sprite::ModeHeight[mode]); value < end; ++value)
                {
                    std::list<Sprite*>& line = spriteLines_[newPosY];
                    std::list<Sprite*>::iterator it = line.begin();
                    for (std::list<Sprite*>::iterator end = line.end(); it != end && s.x < (*it)->x; ++it);
                    line.insert(it, &s);
                }

                s.y = newPosY;
                break; }
            case 1: {
                // posx
                unsigned int ssize = Sprite::ModeHeight[mode];
                s.x = value;
                for (unsigned int i = 0; i < ssize; ++i)
                {
                    std::list<Sprite*>& line = spriteLines_[s.y + i];
                    line.remove(&s);
                    std::list<Sprite*>::iterator it, end;
                    for (it = line.begin(), end = line.end();
                        it != end; ++it)
                    {
                        unsigned int otherx = (*it)->x;
                        if (s.x > otherx || (s.x == otherx && &s > *it))
                            break;
                    }
                    line.insert(it, &s);
                }
                break; }
            case 2:
                // patternId
                s.setPixels(spritePatternTable_.getPattern(value, palettes_[attr.palette], mode));
                break;
            case 3: {
                // single bit stuff: prio, x/yflip, palette
                static const unsigned int prio = 1 << 7;
                static const unsigned int yflip = 1 << 6;
                static const unsigned int xflip = 1 << 5;
                static const unsigned int palette = 1 << 4;
                
                if ((value & yflip) != (oldValue & yflip))
                    s.flipy();
                if ((value & xflip) != (oldValue & xflip))
                    s.flipx();
                if ((value&palette) != (oldValue & palette))
                    s.setPixels(spritePatternTable_.getPattern(value, palettes_[attr.palette], mode));

                break; }
            }
        }

        void VideoSystem::onOAMWritew(unsigned int value, unsigned int oldValue, unsigned int addr)
        {
            onOAMWrite(value & 0xFF, oldValue & 0xFF, addr);
            onOAMWrite((value & 0xFF00) >> 8, (oldValue &0xFF00) >> 8, addr + 1);
        }

        void VideoSystem::setPalette(PaletteId id, const Palette& p)
        {
            palettes_[id] = p;
        }

        void VideoSystem::drawLine()
        {
            unsigned int line = *LY_;
            drawLine(line);
            *LY_ = (line + 1) % ScanLines;
        }

        void VideoSystem::drawAll()
        {
            Sprite s(ScreenWidth, ScreenHeight);
            s.x = 0;
            s.y = 0;
            tilePatternTable_.invalidate();
            buildTileMap_(backgroundMap_, backgroundTable_);

            for (int y = 0; y < ScreenHeight; ++y)
            {
                std::memcpy(&s[y * ScreenWidth], &backgroundMap_[y * fbwidth], ScreenWidth * sizeof(unsigned int));
            }
            driver_.draw(s);
        }

        static inline bool _skipsprite(int x, int y)
        {
          return x == 0 || x >= 160 + 8 || y == 0 || y >= 144+16;
        }

        void VideoSystem::drawLine(int line)
        {
            if (!lcdc_->lcdOp || line > ScreenHeight)
                return;
            if (line == 0)
            {
                buildTileMap_(backgroundMap_, backgroundTable_);
                buildTileMap_(windowMap_, windowTable_);
            }

            Sprite result(ScreenWidth, 1);
            result.x = 0;
            result.y = line;

            if (lcdc_->backgroundDisplay)
            {
                int bckgrd_y = (*SCY_ + line) % ScreenHeight;
                int bckgrd_x = *SCX_;
                unsigned int bckgrd_pixy = bckgrd_y * fbwidth;
                for (unsigned int i = 0; i < ScreenWidth; ++i)
                {
                    unsigned int pixel = backgroundMap_[bckgrd_pixy + ((bckgrd_x + i) % fbwidth)];
                    result.set(i, 0, pixel);
                }
            }

            
            int wx = *WX_ - 7;
            int wy = *WY_;
            if (lcdc_->windowDisplay
                && line >= wy
                && wy >= 0 && wy <= 143
                && wx >= 0 && wx <= 160)
            {
                int winbuf_y = line - wy;
                unsigned int winbuf_pix = winbuf_y * fbwidth;
                for (; wx < ScreenWidth; ++winbuf_pix, ++wx)
                {
                    unsigned int pixel = windowMap_[winbuf_pix];
                    result.set(wx, 0, pixel);
                }
            }
            

            
            Sprite::SizeMode mode = static_cast<Sprite::SizeMode>(lcdc_->spriteSizeMode);
            // todo: faire une map de sprite lines a la ligne 0 pour eviter de refaire tout ca pour chaque ligne
            // todo: gerer attr->priority mais c'est chiant
            std::vector<ObjectAttribute*> orderedSprites;
            orderedSprites.reserve(40);
            for (int i = 0; i < 40; ++i)
            {
                ObjectAttribute& attr = spriteAttrMem_[i];

                if (_skipsprite(attr.posx, attr.posy))
                    continue;

                unsigned int posy = attr.posy - SPRITE_YPOS_ADJUST;
                if (posy > line || posy + Sprite::ModeHeight[mode] <= line)
                    continue;
                orderedSprites.push_back(&attr);
            }
            // We sort from lowest to highest priority : lower prio sprites will
            // hence be overwritten.
            // LOWEST PRIO FIRST
            std::sort(orderedSprites.begin(), orderedSprites.end(), [](ObjectAttribute* a, ObjectAttribute* b)
            {
                if (a->posx != b->posx)
                    return a->posx > b->posx;
                return a > b;
            });

            for (auto attr : orderedSprites)
            {
                int posy = attr->posy - SPRITE_YPOS_ADJUST;
                int posx = attr->posx - SPRITE_XPOS_ADJUST;
                Sprite sprite = spritePatternTable_.getSprite(attr->patternId, palettes_[attr->palette], mode);

                if (attr->xflip)
                    sprite.flipx();
                if (attr->yflip)
                    sprite.flipy();
                
                unsigned int sx = posx < 0 ? -posx : 0;
                unsigned int sy = line - posy;
                posx = std::max(posx, 0);

                for (; sx < sprite.width() && posx < ScreenWidth; ++sx)
                    result.set(posx++, 0, sprite.at(sx, sy));
            }





            driver_.draw(result);
        }

        void VideoSystem::buildTileMap_(std::vector<unsigned int>& map, const TileTable& tileSource)
        {
          for (int y = 0; y < TileRows; y++)
          {
            for (int x=  0; x < TileCols; x++)
            {
              unsigned int tileId = tileSource.tileId(x, y);
              Sprite s = tilePatternTable_.getSprite(tileId, palettes_[PAL_BG]);

              int by = y * TilePixSize;
              int bx = x * TilePixSize;
              unsigned int py = 0;
              for (; py < s.height(); ++by, ++py)
              {
                  std::memcpy(&map[by * fbwidth + bx], &s[py * s.width()], s.width() * sizeof(unsigned int));
              }
            }
          }
        }

    }
}
