#ifndef GBONK_VIDEO_VIDEOSYSTEM_HPP
#define GBONK_VIDEO_VIDEOSYSTEM_HPP

#include <cstdint>

#include "video/TilePatternTable.hpp"
#include "video/TileTable.hpp"

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
            struct ObjectAttribute
            {
                uint8_t posy;
                uint8_t posx;
                uint8_t pattern;
                uint8_t priority : 1;
                uint8_t yflip : 1;
                uint8_t xflip : 1;
                uint8_t palette : 1;
                uint8_t unused : 4;
            };
           

            void setLCDC(uint32_t val);
            void draw();

        private:
            static const unsigned int fbwidth = 256;
            static const unsigned int fbheight = 256;
            static const unsigned int width = 160;
            static const unsigned int height = 144;
            static const unsigned int tiles = 32;
            static const unsigned int tileSize = 8;

            uint8_t* vram_; // 0x8000
            ObjectAttribute* spriteAttrMem_; // 0xFE00

            union LCDC {
                struct {
                    uint32_t lcdOp : 1;
                    uint32_t wdwTileTableAddrMode : 1;
                    uint32_t windowDisplay : 1;
                    uint32_t tilePttrnAddrMode : 1;
                    uint32_t bckgrdTileTableAddrMode : 1;
                    uint32_t spriteSizeMode : 1;
                    uint32_t transparencyMode : 1;
                    uint32_t backgroundDisplay : 1;
                };
                uint8_t value;
            };
            LCDC lcdc_;

            // AKA TileDataTable
            TilePatternTable spritePatternTable_;
            TilePatternTable tilePatternTable_;

            TileTable        backgroundTable_;
            TileTable        windowTable_;

            uint32_t scrollx_;
            uint32_t scrolly_;
            uint32_t wndposx_;
            uint32_t wndposy_;
        };

    }
}

#endif