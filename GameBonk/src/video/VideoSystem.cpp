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
            tilePatternTable_.setAddr(baseAddr + tile_pattern_table_addresses[lcdc_.tilePttrnAddrMode]);
            backgroundTable_.setAddr(baseAddr + background_tile_table_addresses[lcdc_.bckgrdTileTableAddrMode]);
            windowTable_.setAddr(baseAddr + window_tile_table_addresses[lcdc_.wdwTileTableAddrMode]);
        }

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
                        unsigned int tile = windowTable_.tileId(wnpx, wnpy);
                        // tilePattern = tilePatternTable_.get(tile);
                        // draw tilePattern at (scrollx, scrolly)
                        wnpy += tileSize;
                    }
                    wnpx += tileSize;
                }
            }


        }

    }
}