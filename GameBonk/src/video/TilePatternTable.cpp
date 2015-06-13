#include <cassert>

#include "video/TilePatternTable.hpp"

namespace GBonk
{

    namespace Video
    {

        const unsigned int TilePatternTable::spriteModeMask_[2] = {0xFF, 0xFE};


        TilePatternTable::TilePatternTable()
            : addr_(nullptr),
            firstSpriteIdx_(0)
        {
            built_.resize(tileCount_);
            invalidate();
            pixels_.resize(tileCount_ * pixpertile_);
        }

        void TilePatternTable::setAddr(const uint8_t* baseAddr, uint32_t addr_off)
        {
            std::fill(built_.begin(), built_.end(), false);
            if (addr_off == 0x8800)
                firstSpriteIdx_ = 0x80;
            else
                firstSpriteIdx_ = 0;
            addr_ = baseAddr + addr_off;
        }

        void TilePatternTable::invalidate()
        {
            std::fill(built_.begin(), built_.end(), false);
        }

        Sprite TilePatternTable::getSprite(int tileId, const Palette& p, Sprite::SizeMode spriteMode_)
        {
            int tileidx = (firstSpriteIdx_ + tileId) & spriteModeMask_[spriteMode_];

            assert(tileidx < 0xFF && tileidx >= 0);
            if (built_[tileidx] == false)
            {
                buildSprite_(tileidx);
                if (spriteMode_ == Sprite::_8x16 && !built_[tileidx + 1])
                    buildSprite_(tileidx + 1);
            }
            
            // Sprites are always 8 pixels wide
            Sprite result(8, Sprite::ModeHeight[spriteMode_]);
            int pidx = tileidx * pixpertile_;
            
            for (unsigned int y = 0; y < Sprite::ModeHeight[spriteMode_]; ++y)
            {
                for (int x = 0; x < 8; ++x)
                {
                    result.set(x, y, p[pixels_[pidx++]]);
                }
            }

            return result;
        }

        void TilePatternTable::buildSprite_(int idx)
        {
            const uint8_t* patternStart = addr_ + (tileSize_ * idx);
            unsigned int pixIdx = idx * pixpertile_;
            
            for (int patternIdx = 0; patternIdx < tileSize_; patternIdx += 2)
            {
                for (int bit = 7; bit >= 0; bit--)
                {
                    unsigned int colorId = ((patternStart[patternIdx] & (1 << bit)) >> bit)
                        | ((patternStart[patternIdx + 1] & (1 << bit)) >> bit) << 1;
                    pixels_[pixIdx++] = colorId;
                }
            }
            built_[idx] = true;
        }
    }


}
