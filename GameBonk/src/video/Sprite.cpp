#include <cstdint>

#include "video/Sprite.hpp"

namespace GBonk
{
    namespace Video
    {
        Sprite::Sprite(uint32_t* mem, unsigned int w, unsigned int h)
            : pixels_(mem),
            w_(w),
            h_(h)
        {
        }

        uint32_t Sprite::at(int x, int y) const
        {
            return pixels_[y * w_ + x];
        }
    }
}