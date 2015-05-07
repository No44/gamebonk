#ifndef GBONK_VIDEO_VIDEOSYSTEM_HPP
#define GBONK_VIDEO_VIDEOSYSTEM_HPP

#include <array>
#include <cstdint>

#include "video/TilePatternTable.hpp"
#include "video/TileTable.hpp"
#include "video/Sprite.hpp"

namespace GBonk
{

    namespace Video
    {



        class VideoSystem
        {
        public:
            VideoSystem(uint8_t* vram, uint8_t* spriteAttribMemory);

            enum Transparency
            {
                SOLID,
                TRANSPARENT,
            };
           

            void setLCDC(uint32_t val);
            void drawline(int line);

        private:
            void buildBackground_();

            static const unsigned int fbwidth = 256;
            static const unsigned int fbheight = 256;
            static const unsigned int width = 160;
            static const unsigned int height = 144;
            static const unsigned int TileRows = 32;
            static const unsigned int TileCols = 32;
            static const unsigned int TilePixSize = 8;
            static const unsigned int SPRITE_XPOS_ADJUST = 8;
            static const unsigned int SPRITE_YPOS_ADJUST = 16;

            uint8_t* vram_; // 0x8000
            ObjectAttribute* spriteAttrMem_; // 0xFE00

            union LCDC {
                struct {
                    uint8_t lcdOp : 1;
                    uint8_t wdwTileTableAddrMode : 1;
                    uint8_t windowDisplay : 1;
                    uint8_t tilePttrnAddrMode : 1;
                    uint8_t bckgrdTileTableAddrMode : 1;
                    uint8_t spriteSizeMode : 1;
                    uint8_t transparencyMode : 1;
                    uint8_t backgroundDisplay : 1;
                };
                uint8_t value;
            };
            LCDC lcdc_;

            // AKA TileDataTable
            TilePatternTable spritePatternTable_;
            TilePatternTable tilePatternTable_;

            TileTable        backgroundTable_;
            TileTable        windowTable_;

            Palette palettes_[2];

            uint32_t scrollx_;
            uint32_t scrolly_;
            uint32_t wndposx_;
            uint32_t wndposy_;

            std::array<unsigned int, 256*256> backgroundMap_;
        };

    }
}

#endif
