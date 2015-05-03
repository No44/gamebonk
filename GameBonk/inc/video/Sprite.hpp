#ifndef _GBONK_VIDEO_SPRITE_HPP_
#define _GBONK_VIDEO_SPRITE_HPP_

#include <cstdint>

namespace GBonk
{

    namespace Video
    {

        class Sprite
        {
        public:
            Sprite(uint32_t* mem, unsigned int width, unsigned int height);
            unsigned int width() { return w_; }
            unsigned int height() { return h_; }

            uint32_t at(int x, int y) const;

        private:
            uint32_t* pixels_;
            unsigned int w_;
            unsigned int h_;
        };


    }


}

#endif