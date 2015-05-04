#include <cassert>

#include "video/TilePatternTable.hpp"

namespace GBonk
{

    namespace Video
    {

        TilePatternTable::TilePatternTable()
            : addr_(nullptr),
            firstSpriteIdx_(0),
            modeW_(8),
            modeH_(8),
            spriteMode_(_8x8)
        {
            built_.resize(tileCount_);
            std::fill(built_.begin(), built_.end(), false);
            pixels_.resize(tileCount_ * pixpertile_);
        }

        void TilePatternTable::setAddr(const uint8_t* baseAddr, uint32_t addr_off)
        {
            std::fill(built_.begin(), built_.end(), false);
            if (addr_off == 0x8800)
                firstSpriteIdx_ = 128;
            else
                firstSpriteIdx_ = 0;
            addr_ = baseAddr + addr_off;
        }

        Sprite TilePatternTable::getSprite(int tileId, const Palette& p)
        {
            int tileidx = (firstSpriteIdx_ + tileId) & spriteMode_;

            assert(tileidx <= 0xFF);
            if (!built_[tileidx] == 0)
            {
                buildSprite_(tileidx);
                if (spriteMode_ == _8x16 && !built_[tileidx + 1])
                    buildSprite_(tileidx + 1);
            }
            
            Sprite result(&pixels_[tileidx * pixpertile_], modeW_, modeH_);
            for (unsigned int y = 0; y < modeH_; y++)
            {
                unsigned int line = y * 8;
                for (unsigned int x = 0; x < modeW_; x++)
                    result[line + x] = p[result[line + x]];
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
                    pixels_[pixIdx] = colorId;
                }
            }
            built_[idx] = true;
        }
    }


}