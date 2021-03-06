#ifndef GBONK_VIDEO_VIDEOSYSTEM_HPP
#define GBONK_VIDEO_VIDEOSYSTEM_HPP

#include <cstdint>
#include <map>
#include <list>

#include "video/Driver.hpp"
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
            VideoSystem(uint8_t* baseMem);

            void updateLCDC();

            void render();
            void forcePumpEvents();

            void drawAll();

            // todo: a opti
            void drawLine();
            void drawLine(int line);

            void onVramWrite(unsigned int value, unsigned int addr);
            void onVramWritew(unsigned int value, unsigned int addr);
            void onOAMWrite(unsigned int value, unsigned int previousValue, unsigned int addr);
            void onOAMWritew(unsigned int value, unsigned int previousValue, unsigned int addr);

            enum PaletteId
            {
                PAL_OBJ0 = 0,
                PAL_OBJ1,
                PAL_BG,
                __PAL_COUNT,
            };
            void setPalette(PaletteId, const Palette&);

            Driver driver_;

            static const unsigned int ScreenWidth = 160;
            static const unsigned int ScreenHeight = 144;
            static const unsigned int ScanLines = 153;
            static const unsigned int SpriteCount = 40;

        private:
            void buildTileMap_(std::vector<unsigned int>& map, const TileTable& tileSource);

            static const unsigned int fbwidth = 256;
            static const unsigned int fbheight = 256;
            static const unsigned int TileRows = 32;
            static const unsigned int TileCols = 32;
            static const unsigned int TilePixSize = 8;
            static const unsigned int SPRITE_XPOS_ADJUST = 8;
            static const unsigned int SPRITE_YPOS_ADJUST = 16;

            uint8_t* baseMem_;
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

            LCDC* lcdc_;


            uint8_t* SCY_;
            uint8_t* SCX_;
            uint8_t* WX_;
            uint8_t* WY_;
            uint8_t* LY_;
            uint8_t* LYC_;

            // AKA TileDataTable
            TilePatternTable spritePatternTable_;
            TilePatternTable tilePatternTable_;

            TileTable        backgroundTable_;
            TileTable        windowTable_;

            std::map<int, std::list<Sprite*>> spriteLines_;
            std::vector<Sprite> sprites_;

            Palette palettes_[__PAL_COUNT];

            std::vector<unsigned int> backgroundMap_;
            std::vector<unsigned int> windowMap_;
        };

    }
}

#endif
