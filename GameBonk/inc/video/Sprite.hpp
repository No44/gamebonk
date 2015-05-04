#ifndef _GBONK_VIDEO_SPRITE_HPP_
#define _GBONK_VIDEO_SPRITE_HPP_

#include <cstdint>
#include <array>

namespace GBonk
{

    namespace Video
    {

        typedef std::array<unsigned int, 4> Palette;

        class Sprite
        {
        public:
            struct Attributes
            {
                uint8_t posX;
                uint8_t posY;
                uint8_t patternId;
                uint8_t priority : 1;
                uint8_t yflip : 1;
                uint8_t xflip : 1;
                uint8_t palette : 1;
                uint8_t ___unused : 4;
            };

            Sprite(const uint32_t* mem, unsigned int width, unsigned int height);
            Sprite(Sprite&&);
            ~Sprite();
            unsigned int width() { return w_; }
            unsigned int height() { return h_; }

            
            uint32_t& operator[](int idx) { return pixels_[idx]; }
            uint32_t operator[](int idx) const { return pixels_[idx]; }
            uint32_t at(int x, int y) const;

        private:
            uint32_t* pixels_;
            unsigned int w_;
            unsigned int h_;
        };


    }


}

#endif