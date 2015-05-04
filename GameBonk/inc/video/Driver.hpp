#ifndef GBONK_VIDEO_DRIVER_HPP
#define GBONK_VIDEO_DRIVER_HPP

#include "video/Sprite.hpp"

namespace GBonk
{

    namespace Video
    {

        class Driver
        {
        public:

            void draw(const Sprite&, const Sprite::Attributes&);

        private:
        };

    }

}


#endif