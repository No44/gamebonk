#include <cstring>
#include <cstdint>
#include <utility>

#include "video/Sprite.hpp"

namespace GBonk
{
    namespace Video
    {

        const unsigned int Sprite::ModeHeight[2] = { 8, 16 };

        Sprite::Sprite()
          : pixels_(),
            w_(8),
            h_(8)
        {

        }

        Sprite::Sprite(const uint32_t* mem, unsigned int w, unsigned int h)
            : pixels_(),
            w_(w),
            h_(h)
        {
            pixels_.resize(w * h);
            std::memcpy(&pixels_[0], mem, w * h * sizeof(uint32_t));
        }

        Sprite::Sprite(unsigned int w, unsigned int h)
            : pixels_(),
            w_(w),
            h_(h)
        {
            pixels_.resize(w*h);
            for (unsigned int i = 0; i < w*h; ++i)
                pixels_[i] = 0x000000FF;
        }

        Sprite::Sprite(Sprite&& s)
            : pixels_(s.pixels_),
            w_(s.w_),
            h_(s.h_)
        {
            s.pixels_ = std::vector<uint32_t>();
            s.w_ = 0;
            s.h_ = 0;
        }

        Sprite::~Sprite()
        {
            pixels_.clear();
        }

        void Sprite::setPixels(const std::vector<uint32_t>& pixels)
        {
            pixels_ = pixels;
        }

        void Sprite::setPixels(std::vector<uint32_t>&& pixels)
        {
            pixels_ = pixels;
        }

        void Sprite::setPixels(const std::vector<uint32_t>& pixels, unsigned int nw, unsigned int nh)
        {
            pixels_ = pixels;
            w_ = nw;
            h_ = nh;
        }

        Sprite& Sprite::operator=(Sprite&& s)
        {
          if (&s == this)
            return *this;
          pixels_ = s.pixels_;
          w_ = s.w_;
          h_ = s.h_;
          s.pixels_ = std::vector<uint32_t>();
          s.w_ = 0;
          s.h_ = 0;
          return *this;
        }

        uint32_t Sprite::at(int x, int y) const
        {
            return pixels_[y * w_ + x];
        }

        void Sprite::flipx()
        {
          for (unsigned int y = 0; y < h_; ++y)
          {
            uint32_t* line = &pixels_[y * w_];
            for (int i = 0, j = w_ - 1; i < j; ++i, --j)
              std::swap(line[i], line[j]);
          }
        }

        void Sprite::flipy()
        {
          for (int i = 0, j = h_ - 1; i < j; ++i, --j)
          {
            uint32_t* lowline = &pixels_[i * w_];
            uint32_t* highline = &pixels_[j * w_];
            for (unsigned int x = 0; x < w_; ++x)
              std::swap(lowline[x], highline[x]);
          }
        }

    }
}
