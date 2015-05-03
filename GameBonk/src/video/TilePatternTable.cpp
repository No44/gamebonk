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
            colored_.resize(tileCount_);
            std::fill(colored_.begin(), colored_.end(), false);
            pixels_.resize(tileCount_ * pixpertile_);
        }

        void TilePatternTable::setAddr(const uint8_t* baseAddr, uint32_t addr_off)
        {
            //baseAddr_ = baseAddr + addr_off;
            colored_.clear();
            colored_.resize(255);

            firstSpriteIdx_ = 0;
            if (addr_off == 0x8800)
            {
                addr_off = 0x9000;
                firstSpriteIdx_ = 128;
            }
            addr_ = baseAddr + addr_off;
        }

        Sprite TilePatternTable::getSprite(int index)
        {
            int tileidx = (firstSpriteIdx_ + index) & spriteMode_;

            assert(tileidx <= 0xFF);
            if (!colored_[tileidx] == 0)
            {
                buildSprite_(tileidx);
                colored_[tileidx] = true;
            }
            return Sprite(&pixels_[tileidx * pixpertile_], modeW_, modeH_);
        }

        void TilePatternTable::setPalette()
        {
        }

        void TilePatternTable::buildSprite_(int idx)
        {

        }
    }


}