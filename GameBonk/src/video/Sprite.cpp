#include <cstdint>

#include "video/Sprite.hpp"

namespace GBonk
{
    namespace Video
    {
        Sprite::Sprite(const uint32_t* mem, unsigned int w, unsigned int h)
            : pixels_(new uint32_t[w * h]),
            w_(w),
            h_(h)
        {
            std::memcpy(pixels_, mem, w*h*sizeof(*pixels_));
        }

        Sprite::Sprite(Sprite&& s)
            : pixels_(s.pixels_),
            w_(s.w_),
            h_(s.h_)
        {
            s.pixels_ = nullptr;
            s.w_ = 0;
            s.h_ = 0;
        }

        Sprite::~Sprite()
        {
            delete[] pixels_;
            pixels_ = nullptr;
        }

        uint32_t Sprite::at(int x, int y) const
        {
            return pixels_[y * w_ + x];
        }
    }
}