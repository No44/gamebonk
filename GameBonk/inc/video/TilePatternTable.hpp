#ifndef _GBONK_VIDEO_TILEPATTERNTABLE_HPP
#define _GBONK_VIDEO_TILEPATTERNTABLE_HPP

#include <cstdint>
#include <vector>
#include <array>

#include "video/Sprite.hpp"

namespace GBonk
{

    namespace Video
    {

        class TilePatternTable
        {
        public:
            enum SpriteMode
            {
                _8x16 = 0xFE, // 8x16 ignores least significant bit and treats it as 0
                _8x8 = 0xFF
            };

            TilePatternTable();
            void setAddr(const uint8_t* baseAddr, uint32_t taddr_offset);
            Sprite getSprite(int tileId, const Palette&);
            void setPalette();

        private:
            static const unsigned int tileSize_ = 16;
            static const unsigned int pixpertile_ = 64;
            static const unsigned int tileCount_ = 255;

            void buildSprite_(int idx);

            const uint8_t* addr_;
            std::vector<bool> built_;
            std::vector<uint32_t> pixels_;
            
            int firstSpriteIdx_;
            int modeW_;
            int modeH_;
            SpriteMode spriteMode_;
            
        };


    }

}

#endif
