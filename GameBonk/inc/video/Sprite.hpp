#ifndef _GBONK_VIDEO_SPRITE_HPP_
#define _GBONK_VIDEO_SPRITE_HPP_

#include <cstdint>
#include <array>
#include <vector>

namespace GBonk
{

    namespace Video
    {

        typedef std::array<unsigned int, 4> Palette;

        struct ObjectAttribute
        {
            uint8_t posy;
            uint8_t posx;
            uint8_t patternId;
            uint8_t priority : 1;
            uint8_t yflip : 1;
            uint8_t xflip : 1;
            uint8_t palette : 1;
            uint8_t ___unused : 4;
        };

        class Sprite
        {
        public:
            Sprite();
            Sprite(const uint32_t* mem, unsigned int width, unsigned int height);
            Sprite(unsigned int width, unsigned int height);
            Sprite(Sprite&&);
            ~Sprite();
            Sprite& operator=(Sprite&&);

            unsigned int width() const { return w_; }
            unsigned int height() const { return h_; }
            int x;
            int y;

            void flipx();
            void flipy();

            uint32_t& operator[](int idx) { return pixels_[idx]; }
            uint32_t operator[](int idx) const { return pixels_[idx]; }
            uint32_t at(int x, int y) const;
            void set(int x, int y, unsigned int value) { pixels_[y*w_ + x] = value; }

        private:
            std::vector<uint32_t> pixels_;
            unsigned int w_;
            unsigned int h_;
        };


    }


}

#endif
