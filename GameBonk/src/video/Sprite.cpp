#include <cstdint>
#include <utility>

#include "video/Sprite.hpp"

namespace GBonk
{
    namespace Video
    {

        Sprite::Sprite()
          :pixels_(nullptr),
            w_(0),
            h_(0)
        {

        }

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

        void Sprite::flipx()
        {
          for (int y = 0; y <= h_; ++y)
          {
            uint32_t* line = pixels_ + (y * w_);
            for (int i = 0, j = w_ - 1; i < j; ++i, --j)
              std::swap(line[i], line[j]);
          }
        }

        void Sprite::flipy()
        {
          for (int i = 0, j = h_ - 1; i < j; ++i, --j)
          {
            uint32_t* lowline = pixels_ + (i * w_);
            uint32_t* highline = pixels_ + (j * w_);
            for (int x = 0; x < w_; ++x)
              std::swap(lowline[x], highline[x]);
          }
        }

    }
}
